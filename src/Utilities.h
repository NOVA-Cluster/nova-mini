#ifndef UTILITIES_H
#define UTILITIES_H

#include <Arduino.h>

void initSafeSerial();
void safeSerialPrintf(const char* format, ...);

#endif // UTILITIES_H
