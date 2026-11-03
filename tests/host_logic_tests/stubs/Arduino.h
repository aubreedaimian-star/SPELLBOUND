#pragma once
// Host-side STUB for syntax/logic checks only. Not the real Arduino core.
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <algorithm>
typedef uint8_t byte;
#define LED_BUILTIN 6
#define OUTPUT 1
#define HIGH 1
#define LOW 0
#define HEX 16
using std::max;
unsigned long millis(); unsigned long micros(); void delay(unsigned long);
void pinMode(int,int); void digitalWrite(int,int);
class String{public:String(const char* s=""){}String(unsigned long,int){}String& operator+=(const String&){return *this;}const char*c_str()const{return "";}void trim(){}bool startsWith(const char*)const{return false;}unsigned length()const{return 0;}};
struct Print{void print(const char*){} void print(char){} void print(float,int=2){} void print(int){} void print(const String&){}
 void println(){} void println(const char*){} void println(float,int=2){} void println(int){} void println(const String&){}};
struct SerialT:Print{void begin(long){} operator bool(){return true;}}; extern SerialT Serial;
