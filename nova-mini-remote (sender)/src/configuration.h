#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Pin Definitions for LEDs and Buttons
#define LED_RED 23
#define LED_GREEN 25
#define LED_BLUE 26
#define LED_YELLOW 27
#define LED_RESET 32

#define BTN_RED 4
#define BTN_GREEN 5
#define BTN_BLUE 13
#define BTN_YELLOW 14
#define BTN_RESET 18

// Game configuration constants
#define SIMONA_INPUT_TIMEOUT_SECONDS 30
#define SIMONA_DEBUG_TIMEOUT 0  // Set to 1 to enable timeout debug messages
#define LEVELS_PER_ROUND_DEFAULT 1

// ESP-NOW configuration
#define MAX_RETRIES 5       // Maximum number of retries for sending ESP-NOW messages
#define ACK_TIMEOUT_MS 12  // Timeout in milliseconds to wait for an acknowledgement

#endif // CONFIGURATION_H
