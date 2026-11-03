// Host-side logic test for firmware/history/03_vespera_threshold_v2.
// Compiles the sketch on a PC with stub libraries and checks pure logic only:
// payload size, colour-wheel range, off frame, and the motion thresholds.
// It does NOT test I2C timing, Wi-Fi, MQTT delivery or the Vespera lamp.
#include <cstdio>
#include "Arduino.h"
#include "Wire.h"
#include "WiFiNINA.h"
SerialT Serial; TwoWire Wire; WiFiClass WiFi;
unsigned long millis(){return 0;} unsigned long micros(){return 0;} void delay(unsigned long){}
void pinMode(int,int){} void digitalWrite(int,int){}
#include "../../firmware/history/03_vespera_threshold_v2/SPELLBOUND_Vespera_Test/SPELLBOUND_Vespera_Test.ino"
static int fails=0;
#define CHECK(c,msg) do{ if(c) printf("PASS %s\n",msg); else {printf("FAIL %s\n",msg); fails++;} }while(0)
int main(){
  CHECK(sizeof(rgbPayload)==216, "payload is 72 LEDs x RGB = 216 bytes");
  int mx=0; for(int p=0;p<256;p++){byte r,g,b; colorWheel(p,r,g,b); mx=std::max<int>(mx,std::max<int>(r,std::max<int>(g,b)));}
  CHECK(mx<=110, "colorWheel never exceeds brightness cap 110");
  makeOff(); bool z=true; for(auto v:rgbPayload) z&=v==0; CHECK(z,"makeOff clears every byte");
  // Still, flat: az = 1 g (16384 LSB), gyro 0
  previousSampleValid=false; Wire.setRaw(0,0,16384,0,0,0); detectMovement();
  Wire.setRaw(0,0,16384,0,0,0); CHECK(!detectMovement(),"stationary sample -> no movement");
  // Gyro just below / above 60 deg/s (131 LSB per deg/s)
  Wire.setRaw(0,0,16384,(int16_t)(59*131),0,0); CHECK(!detectMovement(),"gyro 59 deg/s -> below threshold");
  Wire.setRaw(0,0,16384,(int16_t)(61*131),0,0); CHECK(detectMovement(),"gyro 61 deg/s -> movement");
  // Accel change of 0.40 g on X between consecutive samples
  Wire.setRaw(0,0,16384,0,0,0); detectMovement();
  Wire.setRaw((int16_t)(0.40*16384),0,16384,0,0,0); CHECK(detectMovement(),"accel delta 0.40 g -> movement");
  Wire.setRaw((int16_t)(0.40*16384),0,16384,0,0,0); CHECK(!detectMovement(),"same accel again (delta 0) -> no movement");
  // Steady tilt is ignored (gravity cancels in the delta)
  Wire.setRaw(11585,0,11585,0,0,0); detectMovement(); Wire.setRaw(11585,0,11585,0,0,0);
  CHECK(!detectMovement(),"constant 45-degree tilt -> no movement");
  Wire.ack=2; CHECK(!detectMovement(),"I2C NACK -> returns false (indistinguishable from 'no movement')");
  printf("%s (%d failures)\n", fails?"SOME TESTS FAILED":"ALL HOST LOGIC TESTS PASSED", fails); return fails;
}
