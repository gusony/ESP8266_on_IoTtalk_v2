#include "ArduinoJson.h" // json library
#include "ESP8266TrueRandom.h" // uuid library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include "ESP8266HTTPClient2.h"
#include <EEPROM.h>
#include <PubSubClient.h> // MQTT library

#include "MyEsp8266.h"



//WiFiClient espClient;
//PubSubClient client(espClient);



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

