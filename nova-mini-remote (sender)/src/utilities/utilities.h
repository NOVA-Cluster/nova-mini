#ifndef UTILITIES_H
#define UTILITIES_H

#include <Arduino.h>

// Thread-safe serial print functions
void initSafeSerial();
void safeSerialPrintf(const char* format, ...);

void NovaLogo();
String getLastFourOfMac();
void setLedBrightness(uint8_t led, bool isOn);

#endif // UTILITIES_H
