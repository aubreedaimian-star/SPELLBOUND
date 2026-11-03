#pragma once
// Host-side STUB of Arduino Wire for logic tests only. Not hardware behaviour.
#include "Arduino.h"
struct TwoWire{
  uint8_t buf[14]={0}; int pos=0; uint8_t ack=0;
  void begin(){} void begin(int,int){} void setClock(long){}
  void beginTransmission(uint8_t){} size_t write(uint8_t){return 1;}
  uint8_t endTransmission(bool=true){return ack;}
  uint8_t requestFrom(uint8_t,uint8_t n,uint8_t=1){pos=0;return n;}
  int read(){return buf[pos++];}
  void setRaw(int16_t ax,int16_t ay,int16_t az,int16_t gx,int16_t gy,int16_t gz){
    int16_t v[7]={ax,ay,az,0,gx,gy,gz};
    for(int i=0;i<7;i++){buf[2*i]=(uint16_t)v[i]>>8;buf[2*i+1]=v[i]&0xFF;}}
};
extern TwoWire Wire;
