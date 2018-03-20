/*
 * feature:
 * 1. store wifi info by json format
 * 2. show html
 * 3. send HTTP post to server
 */

 /*
  * when device set up, it need to check wifi first
  * 1. check wifi status(has connecte to wifi or not), WL_CONNECTED->5. ,other->2.
  * 2. check EEPROM (if there are data in EEPROM), return(0)->5. , false->3.
  * 3. start the web server(let user choose ssid and password ), go to 4.
  * 4. connect to wifi ap and store data to EEPROM, go to 5.
  * 5. wifi has connected.
  */

#include <PubSubClient.h> // MQTT library
#include "ArduinoJson.h" // json library
#include "ESP8266TrueRandom.h" // uuid library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include "ESP8266HTTPClient2.h"
#include <EEPROM.h>
#include "MyEsp8266.h"

extern char IoTtalkServerIP[100];
extern ESP8266WebServer server ;
extern WiFiClient espClient;
extern PubSubClient client;
extern uint8_t wifimode ; //1:AP , 0: STA

#define put_url "http://140.113.199.198:9992/"
String idf_list[10] = { "ESP12F_input1" };
String ctrl_i,ctrl_o,d_name;
byte uuidBytes16[16]; // UUIDs in binary form are 16 bytes long
String deviceuuid;
bool new_message = false;
String mqtt_mes = "";
uint8_t at_least_one_idf_connect = 0;
DynamicJsonBuffer JB_CD;  // CD:ctrl data, i need a better name
JsonArray& JA_CD = JB_CD.createArray();

long last_time;

void store(String df_name, String topic, String command){
  DynamicJsonBuffer JB_temp;
  JsonArray& JA_temp = JA_CD.createNestedArray();
  JA_temp.add(df_name);
  JA_temp.add(topic);
  JA_temp.add(command);
  JB_temp.clear();
}
String state_rev(String state, String rev){
  String mes;
  DynamicJsonBuffer JB_temp;
  JsonObject& JO_temp = JB_temp.createObject();
  JO_temp["state"] = state;
  JO_temp["rev"] = rev;
  JO_temp.printTo(mes);
  JB_temp.clear();
  return(mes);
}
void callback(char* topic, byte* payload, unsigned int length) {
  new_message = true;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  mqtt_mes = "";
  for (int i = 0; i < length; i++) {
    mqtt_mes += (char)payload[i];
  }
  Serial.print(mqtt_mes);
  Serial.println();
}
String make_profile(){
  DynamicJsonBuffer JB_PUT_profile;
  JsonObject& JO_PUT_profile = JB_PUT_profile.createObject();

  JsonArray& odf_list = JO_PUT_profile.createNestedArray("odf_list");
  odf_list.add("ESP12F");
  odf_list.createNestedArray().createNestedArray();

  JsonArray& idf_list = JO_PUT_profile.createNestedArray("idf_list");
  idf_list.add("ESP12F");
  idf_list.createNestedArray().createNestedArray();

  JsonObject& profile = JO_PUT_profile.createNestedObject("profile");
  profile["model"] = "ESP12F";
  profile["u_name"] = "null";

  JsonArray& accept_protos = JO_PUT_profile.createNestedArray("accept_protos");
  accept_protos.add("mqtt");

  String result;
  JO_PUT_profile.printTo(result);
  JB_PUT_profile.clear();
  return(result);

}
int dev_register(){
  String url = put_url;
  String Str_PUT_resp;
  String rev;

  //http PUT
  int httpCode;
  HTTPClient http;
  http.begin(url+deviceuuid);
  http.addHeader("Content-Type","application/json");
  httpCode = http.PUT(make_profile());

  Serial.println("url="+url+deviceuuid);

  for(int i = 0; i<3; i++){
    Str_PUT_resp = http.getString();
    Serial.println("PUT resp : "+Str_PUT_resp);

    if(httpCode == 200) {
      Serial.println("HTTP PUT successful\n");
      DynamicJsonBuffer JB_PUT_resp;
      JsonObject& JO_PUT_resp = JB_PUT_resp.parseObject(Str_PUT_resp);
      ctrl_i = JO_PUT_resp["ctrl_chans"][0].as<String>(); Serial.println("ctrl_i:"+ctrl_i);
      ctrl_o = JO_PUT_resp["ctrl_chans"][1].as<String>(); Serial.println("ctrl_o:"+ctrl_o);
      d_name = JO_PUT_resp["name"].as<String>();          Serial.println("d_name:"+d_name);
      rev    = JO_PUT_resp["rev"].as<String>();
      JB_PUT_resp.clear();
      break;
    }
    else if(httpCode !=200){
      delay(100);
      httpCode = http.PUT(make_profile());
    }
  }


  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(deviceuuid.c_str(), ctrl_i.c_str(), 0, true, state_rev("broken",rev).c_str() )){// connect to mqtt server
      Serial.println("connected state : "+(String)client.state());

      if( client.subscribe(ctrl_i.c_str()) ) //not necessary
        Serial.println("ctrl_i subscribe successful!, " + ctrl_i);

      if( client.subscribe(ctrl_o.c_str()) )
        Serial.println("ctil_o subscribe successful!, " + ctrl_o);

      if( client.publish(ctrl_i.c_str(), state_rev("online",rev).c_str()) )
        Serial.println("publish messenger[" +ctrl_i+"]:"+state_rev("online",rev));
      
      Serial.println("Register finish.");
      break;
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  Serial.println("exit");
}
void CtrlHandle(void){
  new_message = false;
  String ok_mes = "{\"state\":\"ok\",\"msg_id\":\"";

  DynamicJsonBuffer JB_temp;  // CD:ctrl data, i need a better name
  JsonObject& JO_temp = JB_temp.parseObject(mqtt_mes);

  DynamicJsonBuffer JB_ok_mes;  // CD:ctrl data, i need a better name
  JsonObject& JO_ok_mes = JB_ok_mes.createObject();

  if(JO_temp.containsKey("command")){
    if( JO_temp["command"].as<String>() == "CONNECT"){
      if(JO_temp.containsKey("odf")){
        store(JO_temp["odf"].as<String>(), JO_temp["topic"].as<String>(),JO_temp["command"].as<String>());
        client.subscribe(JO_temp["topic"].as<String>().c_str());
      }
      else if(JO_temp.containsKey("idf"))
        store(JO_temp["idf"].as<String>(), JO_temp["topic"].as<String>(),JO_temp["command"].as<String>());

      ok_mes += JO_temp["msg_id"].as<String>() + "\"}";   //Serial.println("ok_mes = "+ ok_mes);
      client.publish(ctrl_i.c_str(), ok_mes.c_str());
    }
    else if(JO_temp["command"].as<String>() == "DISCONNECT"){
      
      /* need to unscribe */
      for(int i=0; i< JA_CD.size(); i++)
        if(JA_CD[i][0].as<String>() == JO_temp["idf"].as<String>()  || JA_CD[i][0].as<String>() == JO_temp["odf"].as<String>() ){
          client.unsubscribe(JA_CD[i][1].as<String>().c_str());
          Serial.println("unsubscribe "+JA_CD[i][0].as<String>()+" "+JA_CD[i][1].as<String>());
          break;
        }
      
      String remove_df_name = JO_temp["idf"].as<String>().length() > 0 ? JO_temp["idf"].as<String>() : JO_temp["odf"].as<String>();
      for(int i =0; i<JA_CD.size(); i++)
        if( remove_df_name == JA_CD[i][0].as<String>())
          JA_CD.remove(i);
    }
  }
}
int check_idf(String df_name){
  for(int i =0 ; i<10; i++){
    if(df_name == idf_list[i]){
      return i;
    }
  }
  return -1;
}
void setup() {

  uint8_t mac[6];
  char *wifiSSID, *wifiPASS;

  delay(100);

  EEPROM.begin(512);
  Serial.begin(115200);
  randomSeed(analogRead(0));
  pinMode(LEDPIN, OUTPUT);  // Initialize the BUILTIN_LED pin as an output
  pinMode(CLEARPIN, INPUT_PULLUP);   // GPIO5 : clear EEPROM

  delay(100);
  
  Serial.println(WiFi.status());
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("status != connnected");
    if(read_WiFi_AP_Info(wifiSSID, wifiPASS, &IoTtalkServerIP[0]) == 0) {
      Serial.println(wifiSSID);
      Serial.println(wifiPASS);
      Serial.println((String)IoTtalkServerIP );
      Serial.println("read data from eeprom");
      connect_to_wifi(wifiSSID, wifiPASS);
    }
    else {
      Serial.println("read no data from EEPROM");
      ap_setting();
    }
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("wifi connected");
  }


  client.setServer(IoTtalkServerIP, 1883);
  client.setCallback(callback);

  //generate device uuid
  WiFi.macAddress(mac);
  deviceuuid = ESP8266TrueRandom.uuidToString(mac);
  Serial.println("deviceuuid : " + deviceuuid );
  dev_register();
  last_time = millis();

}
void loop() {
  if (digitalRead(CLEARPIN) == LOW){
      clr_eeprom(0);
  }

  if(!client.loop()) // like mqtt ping ,to make sure the connection between server
    dev_register();

  if(new_message){
    CtrlHandle();
  }

  for(int i=0; i < JA_CD.size() ; i++){
    if( check_idf(JA_CD[i][0].as<String>())>=0  && (millis()-last_time >5000)){
      last_time = millis();
      client.publish(JA_CD[i][1].as<String>().c_str(), ("["+(String)random(0,1000)+"]" ).c_str() );
      Serial.println("publish, "+JA_CD[i][1].as<String>() );
    }
  }
  
  
      

}

