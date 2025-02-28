#include "main.h"

volatile LEDAnimationState currentLEDAnimationState = LED_WAITING;
const int NUM_RELAYS = 8;
RelayTask relayTasks[NUM_RELAYS] = {{false, false, 0, 0}};

// Optionally, you can also define the shift register object here if needed.
// For example:
// ShiftRegister74HC595<NUMBER_OF_SHIFT_REGISTERS> sr(HT74HC595_DATA, HT74HC595_CLOCK, HT74HC595_LATCH);
