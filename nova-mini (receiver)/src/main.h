#pragma once

#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>                   // Added for uint32_t and other Arduino types
#include "configuration.h"             // Added for NUMBER_OF_SHIFT_REGISTERS
#include <ShiftRegister74HC595.h>       // Added for ShiftRegister74HC595 type

// Added function prototypes for relay control.
void triggerRelay(int channel, int duration);
void triggerRelayLong(int channel, int duration);
void disableRelay(int channel); // <-- Added prototype

// Declare initEspNowReceiver so it can be used in setup().
void initEspNowReceiver();

extern void safeSerialPrintf(const char* format, ...);

// Updated LEDAnimationState enum to support SimonaDisplay stages.
enum LEDAnimationState {
    LED_WAITING,
    LED_SEQUENCE_GENERATION,  // For displaySimonaStageSequenceGeneration
    LED_TRANSITION,           // For displaySimonaStageTransition
    LED_INPUT_COLLECTION,     // For displaySimonaStageInputCollection
    LED_VERIFICATION,         // For displaySimonaStageVerification
    LED_GAME_LOST,            // For displaySimonaStageGameLost
    LED_GAME_WIN,             // For displaySimonaStageGameWin
    LED_RESET,                // For displaySimonaStageReset
    LED_ROUND_TRANSITION      // Add new state for round transition
};

extern volatile LEDAnimationState currentLEDAnimationState;

// Shared relay control types and globals:
struct RelayTask {
    bool active;
    bool longActive; // Flag for long duration events.
    uint32_t offTime;
    uint32_t startTime;
};

extern const int NUM_RELAYS;
extern RelayTask relayTasks[];
extern ShiftRegister74HC595<NUMBER_OF_SHIFT_REGISTERS> sr; // NUMBER_OF_SHIFT_REGISTERS assumed defined in configuration.h

#endif // MAIN_H