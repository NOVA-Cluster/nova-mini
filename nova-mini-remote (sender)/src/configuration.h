#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Pin Definitions for LEDs and Buttons
#define LED_RED 23
#define LED_GREEN 25
#define LED_BLUE 26
#define LED_YELLOW 27
#define LED_RESET 32
#define ENABLE_STATUS_LED 33   // Enable status LED

#define BTN_RED 4
#define BTN_GREEN 5
#define BTN_BLUE 13
#define BTN_YELLOW 14
#define BTN_RESET 18

// Timeout and retry configurations for ESP-NOW transmissions.
#define ACK_TIMEOUT_MS 12
#define MAX_RETRIES 5

// Simona game configuration
#define SIMONA_INPUT_TIMEOUT_SECONDS 30  // How much time to wait for input before game over
#define LEVELS_PER_ROUND_DEFAULT 1       // Default levels per round

// Enable input timeout debugging
#define SIMONA_DEBUG_TIMEOUT true

#endif // CONFIGURATION_H
