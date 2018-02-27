/*
 * feature:
 * 1. store wifi info by json format
 * 2. show html
 * 3. send HTTP post to server
 */

#include <PubSubClient.h> // MQTT library
#include "ArduinoJson.h" // json library
#include "ESP8266TrueRandom.h" // uuid library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include "ESP8266HTTPClient2.h"
#include <EEPROM.h>

ESP8266WebServer server ( 80 );
const char* ssid = "Lab117";
const char* password = "pcs54784";
uint8_t wifimode = 1; //1:AP , 0: STA

//DynamicJsonBuffer jsonBuffer;

String ctrl_i,ctrl_o,d_name;

byte uuidBytes16[16]; // UUIDs in binary form are 16 bytes long
String deviceuuid;

const char* mqtt_server = "140.113.199.198";
char IoTtalkServerIP[100] = "";
WiFiClient espClient;
PubSubClient client(espClient);
//long lastMsg = 0;
//char msg[50];
//int value = 0;


void clr_eeprom(int sw=0){
    if (!sw){
        Serial.println("Count down 3 seconds to clear EEPROM.");
        delay(3000);
    }
    if( (digitalRead(5) == LOW) || (sw == 1) ){
        for(int addr=0; addr<50; addr++) EEPROM.write(addr,0);   // clear eeprom
        EEPROM.commit();
        Serial.println("Clear EEPROM.");
        digitalWrite(2,HIGH);
    }
}
void save_WiFi_AP_Info(char *wifiSSID, char *wifiPASS, char *ServerIP){  //stoage format: [SSID,PASS,ServerIP]

    char *netInfo[3] = {wifiSSID, wifiPASS, ServerIP};
    int addr=0,i=0,j=0;
    EEPROM.write (addr++,'[');  // the code is equal to (EEPROM.write (addr,'[');  addr=addr+1;)
    for (j=0;j<3;j++){
        i=0;
        while(netInfo[j][i] != '\0') EEPROM.write(addr++,netInfo[j][i++]);
        if(j<2) EEPROM.write(addr++,',');
    }
    EEPROM.write (addr++,']');
    EEPROM.commit();
}
int read_netInfo(char *wifiSSID, char *wifiPASS, char *ServerIP){   // storage format: [SSID,PASS,ServerIP]
    char *netInfo[3] = {wifiSSID, wifiPASS, ServerIP};
    String readdata="";
    int addr=0;
  
    char temp = EEPROM.read(addr++);
    if(temp != '['){
      Serial.println("no data in eeprom");
      return 1;
    }
    
    for (int i=0; i<3; i++,readdata =""){
        while(1){
            temp = EEPROM.read(addr++);
            if (temp == ',' || temp == ']') 
              break;
            readdata += temp;
        }
        readdata.toCharArray(netInfo[i],100);
    }

    if (String(ServerIP).length () < 7){
      Serial.println("ServerIP loading failed.");
      return 2;  
    }
    
    Serial.println("Load setting successfully.");
    return 0;
  
    
}
String scan_network(void){
    int AP_N,i;  //AP_N: AP number 
    String AP_List="<select name=\"SSID\" style=\"width: 150px;\">" ;// make ap_name in a string
    AP_List += "<option value=\"\">請選擇</option>";
  
    WiFi.disconnect();
    delay(100);
    AP_N = WiFi.scanNetworks();

    if(AP_N>0) for (i=0;i<AP_N;i++) AP_List += "<option value=\""+WiFi.SSID(i)+"\">" + WiFi.SSID(i) + "</option>";
    else AP_List = "<option value=\"\">NO AP</option>";
    AP_List +="</select><br><br>";
    return(AP_List); 
}
void handleRoot(){
  String temp = "<html><title>Wi-Fi Setting</title>";
  temp += "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>";
  temp += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head>";
  temp += "<form action=\"setup\"><div>";
  temp += "SSID:<br>";
  temp += scan_network();
  temp += "Password:<br>";
  temp += "<input type=\"password\" name=\"Password\" vplaceholder=\"輸入AP密碼\" style=\"width: 150px;\">";
  temp += "<br><br>IoTtalk Server IP<br>";  
  temp += "<input type=\"serverIP\" name=\"serverIP\" value=\"140.113.199.199\" style=\"width: 150px;\">";
  temp += "<br><br><input type=\"submit\" value=\"Submit\" on_click=\"javascript:alert('TEST');\">";
  temp += "</div></form><br>";
  temp += "<div><input type=\"button\" value=\"開啟\" onclick=\"location.href=\'turn_on_pin\'\"><br>";
  temp += "<input type=\"button\" value=\"關閉\" onclick=\"location.href=\'turn_off_pin\'\"><br></div>";
  temp += "</html>";
  server.send ( 200, "text/html", temp );
}
void handleNotFound() {
  Serial.println("Page Not Found ");
  server.send( 404, "text/html", "Page not found.");
}
void saveInfoAndConnectToWiFi() {
    Serial.println("Get network information.");
    char _SSID_[100]="";
    char _PASS_[100]="";
    
    if (server.arg(0) != ""){//arg[0]-> SSID, arg[1]-> password (both string)
      server.arg(0).toCharArray(_SSID_,100);
      server.arg(1).toCharArray(_PASS_,100);
      server.arg(2).toCharArray(IoTtalkServerIP,100);
      server.send(200, "text/html", "ok");
      server.stop();
      save_WiFi_AP_Info(_SSID_, _PASS_, IoTtalkServerIP);
      connect_to_wifi(_SSID_, _PASS_);      
    }
}
void start_web_server(void){
    server.on ( "/", handleRoot );
    server.on ( "/setup", saveInfoAndConnectToWiFi);
    server.onNotFound ( handleNotFound );
    server.begin();  
}
void wifi_setting(void){
    String softapname = "UV_Light";
    IPAddress ip(192,168,0,1);
    IPAddress gateway(192,168,0,1);
    IPAddress subnet(255,255,255,0);  
    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect();
    WiFi.softAPConfig(ip,gateway,subnet);
    WiFi.softAP(&softapname[0]);
    //if ( MDNS.begin ( "esp8266" ) ) Serial.println ( "MDNS responder started" ); //enable Multicast DNS to provide Bonjour service.
    start_web_server();
    Serial.println ( "Switch to AP mode and start web server." );
}
void connect_to_wifi(char *wifiSSID, char *wifiPASS){
  long connecttimeout = millis();
  
  WiFi.softAPdisconnect(true);
  Serial.println("-----Connect to Wi-Fi-----");
  WiFi.begin(wifiSSID, wifiPASS);
  
  while (WiFi.status() != WL_CONNECTED && (millis() - connecttimeout < 10000) ) {
      delay(1000);
      Serial.print(".");
  }
  
  if(WiFi.status() == WL_CONNECTED){
    Serial.println ( "Connected!\n");
    digitalWrite(2,LOW);
    wifimode = 0;
  }
  else if (millis() - connecttimeout > 10000){
    Serial.println("Connect fail");
    wifi_setting();
  }
}
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
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
  String url = "http://140.113.199.198:9992/";
  String Str_PUT_profile = make_profile();//= "{\"odf_list\": [[\"ESP12F\", [null]]], \"profile\": {\"model\": \"ESP12F\", \"u_name\": null}, \"idf_list\": [[\"ESP12F\", [null]]], \"accept_protos\": [\"mqtt\"]}";
  String Str_PUT_resp;
  String rev;
  
  //http PUT
  int httpCode;
  HTTPClient http;
  http.begin(url+deviceuuid);
  http.addHeader("Content-Type","application/json");
  httpCode = http.PUT(Str_PUT_profile);
  
  Serial.println("url="+url+deviceuuid);
  
  for(int i = 0; i<3; i++){
    Str_PUT_resp = http.getString();
    Serial.println("response : "+Str_PUT_resp);

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
      httpCode = http.PUT(Str_PUT_profile);
    }
  }


  while (!client.connected()) {
    DynamicJsonBuffer JB_wm;
    JsonObject& JO_wm = JB_wm.createObject(); //json will messenger
    JO_wm["state"] = "broken";
    JO_wm["rev"] = rev;
    String Str_wm;
    JO_wm.printTo(Str_wm);
    JB_wm.clear();
    
    Serial.print("Attempting MQTT connection...");
    if (client.connect(deviceuuid.c_str(), ctrl_i.c_str(), 0, true, Str_wm.c_str() )){// connect to mqtt server
      Serial.println("connected state : "+(String)client.state());

      if( client.subscribe(ctrl_i.c_str()) )
        Serial.println("ctrl_i subscribe successful!");
      
      if( client.subscribe(ctrl_o.c_str()) )
        Serial.println("ctil_o subscribe successful!");
      
      String mes;
      DynamicJsonBuffer JB_temp;
      JsonObject& JO_temp = JB_temp.createObject();
      JO_temp["state"] = "online";
      JO_temp["rev"] = rev;
      JO_temp.printTo(mes);
      Serial.println("mes = "+mes);

      if( client.publish(ctrl_i.c_str(), mes.c_str()) )
        Serial.println("publish messenger");

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

void setup() {
  //pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  uint8_t mac[6];
  WiFi.macAddress(mac);
  
  //generate device uuid
  deviceuuid = ESP8266TrueRandom.uuidToString(mac);
  Serial.print("deviceuuid : ");
  Serial.println(deviceuuid);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  dev_register();
  
}
void loop() {
  
  if(!client.loop()) // like mqtt ping ,to make sure the connection between server
    dev_register();

  
    
}
