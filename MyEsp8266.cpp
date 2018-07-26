#ifndef all_header
#define all_header
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <PubSubClient.h> // MQTT library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include <SoftwareSerial.h>
#include "ArduinoJson.h" // json library
#include "ESP8266TrueRandom.h" // uuid library
#include "ESP8266HTTPClient.h"
#include "MyEsp8266.h"
#endif

//#define SSD1306_IIC
//#ifdef SSD1306_IIC  //SSD1306 with IIC
//Adafruit_SSD1306 display(OLED_RESET);
//#endif

//char IoTtalkServerIP[100] = "140.113.199.222"; // v1
char IoTtalkServerIP[100] = "140.113.199.198"; // v2
//char IoTtalkServerIP[100] = "140.113.215.7"; // v2

ESP8266WebServer server ( 80 );
WiFiClient espClient;
PubSubClient client(espClient);
uint8_t wifimode = 1; //1:AP , 0: STA

SoftwareSerial pms(PMS_RX, PMS_TX);
SoftwareSerial GPS(GPS_RX, GPS_TX);


//EEPROM//EEPROM
void clr_eeprom(int sw)
{//clear eeprom (and wifi disconnect?)
  if (!sw){
    delay(3000);
  }
  if( (digitalRead(CLEAREEPROM) == LOW) || (sw == 1) ){
    for(int addr=0; addr<512; addr++) EEPROM.write(addr,0);   // clear eeprom
    EEPROM.commit();
    Serial.println("Clear EEPROM.");
    delay(50);
  }
}
void save_WiFi_AP_Info(char *wifiSSID, char *wifiPASS, char *ServerIP)  //stoage format: [SSID,PASS,ServerIP]
{
    char *netInfo[3] = {wifiSSID, wifiPASS, ServerIP};
    int addr=0,i=0,j=0;

    EEPROM.write (addr++,'[');  // the code is equal to (EEPROM.write (addr,'[');  addr=addr+1;)
    for (j=0;j<3;j++){
        i=0;
        while(netInfo[j][i] != '\0') {
          EEPROM.write(addr++,netInfo[j][i++]);
          delay(4); //a eeprom write command need 3.3ms
        }
        if(j<2){
          EEPROM.write(addr++,',');
        }
    }
    EEPROM.write (addr++,']');
    EEPROM.commit();
    delay(50);
}
int  read_WiFi_AP_Info(char *wifiSSID, char *wifiPASS, char *ServerIP) // storage format: [SSID,PASS,ServerIP]
{
    char *netInfo[3] = {wifiSSID, wifiPASS, ServerIP};
    String readdata="";
    int addr=0;
    ////OLED_print("Read EEPROM data");
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

//server ,ap mode//server ,ap mode
String scan_network(void)
{
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
void handleRoot(void)
{
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
  temp += "<input type=\"serverIP\" name=\"serverIP\" value=\""+String(IoTtalkServerIP)+"\" style=\"width: 150px;\">";
  temp += "<br><br><input type=\"submit\" value=\"Submit\" on_click=\"javascript:alert('TEST');\">";
  temp += "</div></form><br>";
  //temp += "<div><input type=\"button\" value=\"開啟\" onclick=\"location.href=\'turn_on_pin\'\"><br>";
  //temp += "<input type=\"button\" value=\"關閉\" onclick=\"location.href=\'turn_off_pin\'\"><br></div>";
  temp += "</html>";
  server.send ( 200, "text/html", temp );
}
void handleNotFound(void)
{
  Serial.println("Page Not Found ");
  server.send( 404, "text/html", "Page not found.");
}
void start_web_server(void)
{
    server.on ( "/", handleRoot );
    server.on ( "/setup", saveInfoAndConnectToWiFi);
    server.onNotFound ( handleNotFound );
    server.begin();
    //OLED_print("Web Server Start!");
}
void ap_setting(void)
{
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
void connect_to_wifi(char *wifiSSID, char *wifiPASS)
{
  long connecttimeout = millis();
  //OLED_print("Connect to Wi-Fi");
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
void saveInfoAndConnectToWiFi(void)
{
    Serial.println("Get network information.");
    char _SSID_[100]="";
    char _PASS_[100]="";

    String temp = "Receive SSID & PASSWORD";
    server.send ( 200, "text/html", temp );

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
      //OLED_print("User keyin ssid\nConnect to "+(String)_SSID_);
      save_WiFi_AP_Info(_SSID_, _PASS_, IoTtalkServerIP);
      connect_to_wifi(_SSID_, _PASS_);
    }
}

//void init_ssd1306(void)
//{
//  display.begin(SSD1306_SWITCHCAPVCC);
//  display.clearDisplay();
//  delay(1000);
//  display.setTextSize(1); //21 char in one line with Textsize == 1 ,10 char with size 2
//  display.setTextColor(WHITE);
//  display.display();
//}
//void OLED_print(String mes)
//{
//  display.clearDisplay();
//  display.setCursor(0,0);
//  display.print(mes);
//  display.display();
//}

//String read_pm25(void) //get pm2.5 data
//{
//    unsigned char pms5003[2];//store pms5003 data
//    long read_timeout;
//    int i;
//    pms.begin(pms_baudrate);
//    read_timeout = millis();
//    
//    while(millis() - read_timeout <= 5000){
//        pms.readBytes(pms5003,2);
//        if(pms5003[0]==0x42 || pms5003[1]==0x4d){//尋找每段資料的開頭
//            for(i=0; i<6; i++){
//                pms.readBytes(pms5003,2);
//            }
//            pms.end();
//            return (String)pms5003[1];
//        }
//    }
//    pms.end();
//    return (String)0;
//}
//String get_GPS( String value)
//{
//  long timeout = millis();
//  char c;
//  String result;
//  bool find_flag = 0;
//  int i ,next_comma,count;
//
//  String Time, Date, Date_Time, Lat/* latitude經度*/ ,Lon/*longitude緯度*/;
//  GPS.begin(GPS_baudrate);
//  while(millis() - timeout < 5000 ){
//    c = GPS.read();
//    if(c == '$')
//    {
//      find_flag = 1;
//      result = "";
//    }
//    else if (c == '\n')
//    {
//      find_flag = 0;
//    }
//
//    
//    if(find_flag  == 1)
//    {
//      result += c;
//    }
//    else if (find_flag == 0 && result.indexOf("GPRMC") != -1)
//    {
//      Serial.println("result:"+result);
//      //                1         2 3         4 5          6 7     89      012
//      //result = "$GPRMC,055316.00,A,2478.7051,N,12099.7145,E,0.636,,100518,,,A*74";
//
//      // Time
//      next_comma = result.indexOf(','); // first comma, usually after $GPRMC
//      i = next_comma+1;
//      next_comma = result.indexOf(',',next_comma+1); //2nd comma
//      if (i == next_comma){
//        Serial.println("1 break");
//        break;
//      }
//      count = 0;
//      for ( i ; i < next_comma-3; i++,count++){
//        if(count==2 || count==4)
//          Time += ':';
//        Time += result[i];
//      }
//
//      // Lon
//      next_comma = result.indexOf(',', next_comma+1);//3th comma, After A
//      i = next_comma+1;
//      next_comma = result.indexOf(',', next_comma+1); //4th , after Lon
//      if (i == next_comma){
//        Serial.println("2 break");
//        break;
//      }
//        
//      for (i; i<next_comma ;i++){
//        if(result[i] == '.'){}
//        else if(Lon.length() == 2){
//          Lon += '.';
//          Lon +=result[i];
//        }
//        else{
//          Lon +=result[i];
//        }
//      }
//      
//      
//      //Lat
//      next_comma = result.indexOf(',', next_comma+1); //5th, after N
//      i = next_comma+1;
//      next_comma = result.indexOf(',', next_comma+1); //6th, after Lat
//      if (i == next_comma){
//        Serial.println("3 break");
//        break;
//      }
//      
//      for (i; i<next_comma ;i++){
//        if(result[i] == '.'){}
//        else if(Lat.length() == 3){
//          Lat += '.';
//          Lat +=result[i];
//        }
//        else{
//          Lat +=result[i];
//        }
//      }
//      
//      next_comma = result.indexOf(',', next_comma+1);//7th
//      next_comma = result.indexOf(',', next_comma+1);//8th
//      next_comma = result.indexOf(',', next_comma+1);//9th
//      i = next_comma+1;
//      next_comma = result.indexOf(',', next_comma+1);//10th
//      if (i == next_comma){
//        Serial.println("4 break");
//        break;
//      }
//        
//      for(i; i<next_comma; i++){
//        Date +=result[i];
//      }
//      Date_Time = "\"20"+Date.substring(4,6)+"-"+Date.substring(2,4)+"-"+Date.substring(0,2)+" "+Time+"\"";
//      GPS.end();
//      return(Lon+", "+Lat+", \"user1\", "+value+", "+Date_Time);
//    }
//    delay(5);
//  }
//  GPS.end();
//  return("24.787058, 120.997664, \"user1\", "+value+",\"2018-05-24 18:50:31\"");
//}
//void lcd_print(String Str,int column,int row)
//{
//  lcd.setCursor(column,row);
//  lcd.print(Str);
//}



