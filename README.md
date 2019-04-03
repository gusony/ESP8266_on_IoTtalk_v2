# Contents
- [Pinout](#pinout)
- [Arduino IDE setting](#arduino-ide-setting)
- [Arduino Library](#arduino-library)
- [Set Library Parameter](#set-library-parameter)
- [Feature Work](#feature-work)
- [Variable](#variable)
- [Notices](#notices)


## Pinout
|ESP12F |volt|
|:-----:|:-----:|
|Vcc|3.3v|
|GND|GND|
|CH_PD|3.3V|
|TX|RX|
|RX|TX|
|GPIO0|HISH(LOW when upload)|
|Reset|HIGH(LOW when used)|  
  
<br/>
  
|ENC28J60|Volt|
|:-----:|:-----:|
|Vcc|3.1V~3.6V(3.3V typical)|
|GND|GND|

<br/>
:heavy_exclamation_mark::heavy_exclamation_mark::heavy_exclamation_mark:<br/>
ESP8266 use 3.3V
BUT! you can use a voltage regulator ([ASM1117-3.3](http://www.advanced-monolithic.com/pdf/ds1117.pdf )) 
AMS1117-3.3 Vin=5V, Vout=3.3V<br/>
So you can use 5V voltage.<br/>


## Arduino IDE setting
Type|Parameter
:---:|:---:
Board|Generic ESP8266 Module
Flash Mode|QIO
Flash Frequency|40MHz
CPU Frequency|80MHz
Flash size|4M(1M SPIFFS)
Debug port|disabled
Debug Level|None
Reset Method|ck
Upload Speed|115200


## Arduino Library:
* [pubsubclient v2.6.0](https://github.com/knolleary/pubsubclient)  ( [API](https://pubsubclient.knolleary.net/) )
* [UIPEthernet v2.0.7](https://github.com/UIPEthernet/UIPEthernet)
* [ArduinoJson v5.13.1](https://arduinojson.org/?utm_source=meta&utm_medium=library.properties)


## Set Library Parameter
```
[UIPEthernet/utility/uipethernet-conf.h]
    #define UIP_SOCKET_NUMPACKETS    2
    #define UIP_CONF_MAX_CONNECTIONS 2
    #define UIP_CONF_UDP_CONNS 1 //reduce memory usage
    #define UIP_CONNECT_TIMEOUT 5
[pubsubclient/src/PubSubClient.h]
    #define MQTT_MAX_PACKET_SIZE 512
```


## Feature Work
1. ~~用bs170替換~~
2. ~~寫Init()~~


## Variable
* ServerIP/host : char array, global variable
* ServerPort    : #define , global variable


:bangbang:
## Notices
1.make sure that memory usage and point are used carefully

---
