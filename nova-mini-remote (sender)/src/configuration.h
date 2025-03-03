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

// LEDC (PWM) Configuration
#define LEDC_FREQ_HZ      5000
#define LEDC_RESOLUTION   8       // 8-bit resolution (0-255)
#define LEDC_FULL_DUTY    255     // Full brightness duty cycle
#define LEDC_DIM_DUTY     5      // 10% brightness duty cycle (255 * 0.10)

#define LEDC_CHANNEL_RED      0
#define LEDC_CHANNEL_GREEN    1
#define LEDC_CHANNEL_BLUE     2
#define LEDC_CHANNEL_YELLOW   3
#define LEDC_CHANNEL_RESET    4

// Game configuration constants
#define SIMONA_INPUT_TIMEOUT_SECONDS 30
#define SIMONA_DEBUG_TIMEOUT 0  // Set to 1 to enable timeout debug messages
#define LEVELS_PER_ROUND_DEFAULT 1

// ESP-NOW configuration
#define MAX_RETRIES 5       // Maximum number of retries for sending ESP-NOW messages
#define ACK_TIMEOUT_MS 12  // Timeout in milliseconds to wait for an acknowledgement

// Task reporting interval (matching receiver implementation)
#define REPORT_TASK_INTERVAL (120 * 1000) // How often to report task status in milliseconds

#endif // CONFIGURATION_H
