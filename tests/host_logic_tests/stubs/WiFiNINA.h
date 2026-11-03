#pragma once
#include "Arduino.h"
#define WL_CONNECTED 3
#define WL_NO_MODULE 255
struct IPAddress{}; void operator<<(Print&,IPAddress);
struct WiFiClient:Print{operator bool(){return true;} bool connected(){return false;} void setTimeout(long){} String readStringUntil(char){return String();} void stop(){}};
struct WiFiServer{WiFiServer(int){} void begin(){} WiFiClient available(){return WiFiClient();}};
struct WiFiClass{int status(){return 0;} int begin(const char*,const char*){return 0;} int localIP(){return 0;}}; extern WiFiClass WiFi;
