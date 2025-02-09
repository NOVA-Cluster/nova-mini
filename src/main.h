#pragma once

// Added function prototypes for relay control.
void triggerRelay(int channel, int duration);
void triggerRelayLong(int channel, int duration);
void disableRelay(int channel); // <-- Added prototype

// Declare initEspNowReceiver so it can be used in setup().
void initEspNowReceiver();

// Nothing here yet