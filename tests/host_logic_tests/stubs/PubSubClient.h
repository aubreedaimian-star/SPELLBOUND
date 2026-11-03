#pragma once
#include "WiFiNINA.h"
struct PubSubClient{PubSubClient(WiFiClient&){} void setServer(const char*,uint16_t){} bool setBufferSize(uint16_t){return true;}
 bool connect(const char*,const char*,const char*){return true;} bool connected(){return true;} int state(){return 0;} bool loop(){return true;}
 bool publish(const char*,const uint8_t*,unsigned int){return true;}};
