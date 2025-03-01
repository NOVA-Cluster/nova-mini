#pragma once
#include <esp_now.h>
#include "SimonaMessage.h"  // Replaced forward declaration with actual include

void initEspNowSender();
void espNowSetup();
void espNowLoop();
void sendEspNowMessage();
void sendSimonaMessage(const SimonaMessage &simMsg); // Declaration remains the same

// New function to get packet loss stats
float getPacketLossPercentage();
int getTotalMessagesSent();
int getTotalMessagesLost();