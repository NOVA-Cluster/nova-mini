#pragma once
#include <esp_now.h>

// Message containing 4 poofers info.
struct PooferCommand {
    uint16_t duration[4];
    uint8_t red[4];
    uint8_t green[4];
    uint8_t blue[4];
};

void initEspNowReceiver();
