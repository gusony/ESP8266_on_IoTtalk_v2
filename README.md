pinout
ESP12F  <------->  UART to USB
Vcc     <------->  3.3v
GND     <------->  GND
CH_PD   <------->  HIGH(3.3V)
TX      <------->  RX
RX      <------->  TX

GPIO0   <------->  LOW(GND)  (when upload)

Arduino IDE setting
Board           : Generic ESP8266 Module
Flash Mode      : DIO
Flash Frequency : 40MHz
CPU Frequency   : 80MHz
Flash size      : 4M(1M SPIFFS)
Debug port      : disabled
Debug Level     : None
Reset Method    : ck
Upload Speed    : 115200

Arduino Library:
[pubsubclient](https://github.com/knolleary/pubsubclient)  ( [API](https://pubsubclient.knolleary.net/) )


