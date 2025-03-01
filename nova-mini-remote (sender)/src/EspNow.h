#pragma once
#include <esp_now.h>
#include "SimonaMessage.h"  // Replaced forward declaration with actual include

// Add external variable declaration
extern bool WIRELESS_ENABLED;

void initEspNowSender();
void espNowSetup();
void espNowLoop();
void sendEspNowMessage();
void sendSimonaMessage(const SimonaMessage &simMsg); // Declaration remains the same

// New function to get packet loss stats
float getPacketLossPercentage();
int getTotalMessagesSent();
int getTotalMessagesLost();
int getMessagesInTimeWindow(); // New function to get count in 5-min window