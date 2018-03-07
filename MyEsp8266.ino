#include "ArduinoJson.h" // json library
#include "ESP8266TrueRandom.h" // uuid library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include "ESP8266HTTPClient2.h"
#include <EEPROM.h>
#include <PubSubClient.h> // MQTT library

#include "MyEsp8266.h"



ESP8266WebServer server ( 80 );
WiFiClient espClient;
PubSubClient client(espClient);
uint8_t wifimode = 1; //1:AP , 0: STA



void clr_eeprom(int sw){//clear eeprom (and wifi disconnect?)
    if (!sw){
        //Serial.println("Count down 3 seconds to clear EEPROM.");
        delay(3000);
    }
    if( (digitalRead(CLEARPIN) == LOW) || (sw == 1) ){
        for(int addr=0; addr<50; addr++) EEPROM.write(addr,0);   // clear eeprom
        EEPROM.commit();
        Serial.println("Clear EEPROM.");
        //digitalWrite(LEDPIN,HIGH);
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
int  read_WiFi_AP_Info(char *wifiSSID, char *wifiPASS, char *ServerIP){   // storage format: [SSID,PASS,ServerIP]
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

    //WiFi.disconnect();
    //delay(100);
    AP_N = WiFi.scanNetworks();

    if(AP_N>0)
      for (i=0;i<AP_N;i++)
        AP_List += "<option value=\""+WiFi.SSID(i)+"\">" + WiFi.SSID(i) + "</option>";
    else
      AP_List = "<option value=\"\">NO AP</option>";

    AP_List +="</select><br><br>";
    return(AP_List);
}
void handleRoot(void){
  Serial.println("handleRoot");
  String temp = "<html><title>Wi-Fi Setting</title>";
  temp += "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>";
  temp += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head>";
  temp += "<form action=\"setup\"><div>";
  temp += "SSID:<br>";
  temp += scan_network();
  temp += "Password:<br>";
  temp += "<input type=\"password\" name=\"Password\" vplaceholder=\"輸入AP密碼\" style=\"width: 150px;\">";
  temp += "<br><br>IoTtalk Server IP<br>";
  temp += "<input type=\"serverIP\" name=\"serverIP\" value=\"140.113.199.198\" style=\"width: 150px;\">";
  temp += "<br><br><input type=\"submit\" value=\"Submit\" on_click=\"javascript:alert('TEST');\">";
  temp += "</div></form><br>";
  //temp += "<div><input type=\"button\" value=\"開啟\" onclick=\"location.href=\'turn_on_pin\'\"><br>";
  //temp += "<input type=\"button\" value=\"關閉\" onclick=\"location.href=\'turn_off_pin\'\"><br></div>";
  temp += "</html>";
  server.send ( 200, "text/html", temp );
}
void handleNotFound(void) {
  Serial.println("Page Not Found ");
  server.send( 404, "text/html", "Page not found.");
}
void start_web_server(void){
    server.on ( "/", handleRoot );
    server.on ( "/setup", saveInfoAndConnectToWiFi);
    server.onNotFound ( handleNotFound );
    server.begin();
}
void ap_setting(void){
  String softapname = "ESP12F-";
  byte mac[6];  
  WiFi.macAddress(mac);
  for (int i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%X", mac[i]);
    if(mac[i] < 0x10 )
      softapname += "0";
    softapname += buf;
  }
  Serial.println(softapname);
  
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
  while(wifimode) server.handleClient();
  Serial.println("exit ap_setting");
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
    digitalWrite(LEDPIN,LOW);
    wifimode = 0;
  }
  else if (millis() - connecttimeout > 10000){
    Serial.println("Connect fail");
    ap_setting();
  }
}
void saveInfoAndConnectToWiFi(void) {
    Serial.println("Get network information.");
    char _SSID_[100]="";
    char _PASS_[100]="";

    if (server.arg(0) != ""){//arg[0]-> SSID, arg[1]-> password (both string)
      server.arg(0).toCharArray(_SSID_,sizeof(_SSID_));
      server.arg(1).toCharArray(_PASS_,sizeof(_PASS_));
      server.arg(2).toCharArray(IoTtalkServerIP,100);
      server.send(200, "text/html", "ok");
      server.stop();
      Serial.print("[");
      Serial.print(_SSID_);
      Serial.print("][");
      Serial.print(_PASS_);
      Serial.print("][");
      Serial.print(IoTtalkServerIP);
      Serial.println("]");
      save_WiFi_AP_Info(_SSID_, _PASS_, IoTtalkServerIP);
      connect_to_wifi(_SSID_, _PASS_);
    }
}

