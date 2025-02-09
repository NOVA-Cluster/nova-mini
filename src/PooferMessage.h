#ifndef POOFER_MESSAGE_H
#define POOFER_MESSAGE_H

#include <stdint.h>

typedef struct {
  uint32_t duration;
  uint8_t r;
  uint8_t g;
  uint8_t b;
} PooferData;

typedef struct {
  PooferData poofers[4];
  uint32_t messageId;
} PooferMessage;

extern PooferMessage currentMessage;

#endif
