/*
   my esp8266 wifi function
*/
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

///////////////////pin out////////////////////////////////////////////
#define UPLOAD       0  // when you want to upload code to esp8266, this pin must be LOW
#define LEDPIN       2  // on board led
#define CLEAREEPROM  13 //hold for 5 second , it will erease the contain in eeprom

#define PMS_TX  14 //on nodemcu GPIO14 <---_ PMS_RXpin , no bird use
#define PMS_RX  12 //on nodemcu GPIO12 <---> PMS_TXpin
#define pms_baudrate  9600

#define GPS_TX  5 //on nodemcu GPIO14 <---_ PMS_RXpin , no bird use
#define GPS_RX  4 //on nodemcu GPIO12 <---> PMS_TXpin
#define GPS_baudrate  9600

//#define SSD1306_IIC  //SSC1307 use I2C
#ifdef SSD1306_IIC   
#define OLED_RESET 3 //reset pin will not be used, just for declare
#define OLED_SDA   4 //SDA:04, SCL:05 :those two pin are on board
#define OLED_CLK   5 //but those two define will not be used
#endif

#ifdef  SSD1306_SPI  //SSD1306 with SPI
  #define OLED_MOSI  13   //D1 on SSD1306
  #define OLED_CLK   14   //D0 on SSD1306
  #define OLED_DC    2
  #define OLED_CS    15
  #define OLED_RESET 16
  //Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
#endif
///////////////////pin out////////////////////////////////////////////

//EEPROM
void clr_eeprom(int sw);
void save_WiFi_AP_Info(char *wifiSSID, char *wifiPASS, char *ServerIP);
int  read_WiFi_AP_Info(char *wifiSSID, char *wifiPASS, char *ServerIP);

//server ,ap mode
String scan_network(void);
void handleRoot(void);
void handleNotFound(void);
void start_web_server(void);
void ap_setting(void);

//switch to sta  mode
void connect_to_wifi(char *wifiSSID, char *wifiPASS);
void saveInfoAndConnectToWiFi(void);

//SSD1306, the OLED monitor
//void init_ssd1306(void);
//void OLED_print(String mes);

// sensor (pms3003, pms5003, dht22, dht11 )
//String read_pm25(void);
//String get_GPS( String value);
//void lcd_print(String Str,int column,int row); //print word on lcd,column 0~15,row 0~1










