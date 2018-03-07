/*
   my esp8266 wifi function
*/
#include "ArduinoJson.h" // json library
#include "ESP8266TrueRandom.h" // uuid library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include "ESP8266HTTPClient2.h"
#include <EEPROM.h>

#define LEDPIN 2
#define CLEARPIN 5


void clr_eeprom(int sw);
void save_WiFi_AP_Info(char *wifiSSID, char *wifiPASS, char *ServerIP);
int  read_WiFi_AP_Info(char *wifiSSID, char *wifiPASS, char *ServerIP);

String scan_network(void);
void handleRoot(void);
void handleNotFound(void);
void start_web_server(void);
void ap_setting(void);
void connect_to_wifi(char *wifiSSID, char *wifiPASS);
void saveInfoAndConnectToWiFi(void); 

