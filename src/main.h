#pragma once

// Added function prototypes for relay control.
void triggerRelay(int channel, int duration);
void triggerRelayLong(int channel, int duration);
void disableRelay(int channel); // <-- Added prototype

// Nothing here yet