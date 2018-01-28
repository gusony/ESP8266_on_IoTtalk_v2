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

const char* ssid = "Lab117";
const char* password = "pcs54784";

byte uuidBytes16[16]; // UUIDs in binary form are 16 bytes long
String deviceuuid;

const char* mqtt_server = "140.113.199.198";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

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

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  String temp, willtopic,willmessage,subTopic;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    willtopic = deviceuuid + "/ctrl/i";
    if (client.connect(clientId.c_str())) {

    //if (client.connect(clientId.c_str(), willtopic, 0, 1, willmessage)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      subTopic = deviceuuid + "/ctrl/o";
      client.subscribe(subTopic.c_str());
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int http_regist(){
  String url = "140.113.199.198:9992";
  String Str_profile = "{\"odf_list\": [[\"ESP12F\", [null]]], \"profile\": {\"model\": \"ESP12F\", \"u_name\": null}, \"idf_list\": [[\"ESP12F\", [null]]], \"accept_protos\": [\"mqtt\"]}";
  String PUT_response;
  int httpCode;
  HTTPClient http;
  
  http.begin(url);
  http.addHeader("Content-Type","application/json");
  httpCode = http.PUT(Str_profile);
  if(httpCode !=200){
    Serial.println("error code : "+String(httpCode));
  }
  else {
    Serial.println("put successful");
    PUT_response = http.getString();
    Serial.println("PUT response : "+PUT_response);
  }
  
  
  
  
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);

  //generate device uuid 
  ESP8266TrueRandom.uuid(uuidBytes16);
  deviceuuid = ESP8266TrueRandom.uuidToString(uuidBytes16);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
