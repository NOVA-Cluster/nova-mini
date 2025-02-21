#include <ShiftRegister74HC595.h>
#include "configuration.h" // Renamed include from pin_config.h
#include <iostream>
#include <memory>
#include <Arduino.h>

#include <DNSServer.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <ESPUI.h>
#include "configuration.h"
#include "main.h"                   // Added include for initEspNowReceiver and relay control prototypes
#include "Utilities.h"              // Added include for safeSerialPrintf and initialization
#include <FastLED.h>                // Added support for SM16703 using FastLED
#include "SimonaDisplaySequences.h" // New include for LED animation sequences

#include "Web.h"
#include <stdarg.h>
#include "freertos/semphr.h"  // Corrected include directive

#include "Tasks.h" // New include for task declarations

// Define the number of 74HC595 chips in cascade
// Removed: const uint8_t NUMBER_OF_SHIFT_REGISTERS = 1;

// Create a ShiftRegister74HC595 object using value from configuration.
ShiftRegister74HC595<NUMBER_OF_SHIFT_REGISTERS> sr(HT74HC595_DATA, HT74HC595_CLOCK, HT74HC595_LATCH); // (dataPin, clockPin, latchPin)

void TaskWeb(void *pvParameters);
void TaskOutputs(void *pvParameters);
void TaskPulseRelay(void *pvParameters);
void TaskFastLED(void *pvParameters); // New task for SM16703 LED control using FastLED

DNSServer dnsServer;
AsyncWebServer webServer(80);

// Configurable maximum relay duration in ms
// Removed: const uint32_t MAX_RELAY_DURATION = 250;

// Updated non-blocking relay control implementation

// Function to trigger a relay for a specified duration (in ms), clamped by MAX_RELAY_DURATION.
void triggerRelay(int channel, int duration)
{
    if (channel < 0 || channel >= NUM_RELAYS)
        return;
    sr.set(channel, HIGH);
    relayTasks[channel].active = true;
    relayTasks[channel].longActive = false;
    relayTasks[channel].startTime = millis();
    // Use the requested duration but TaskOutputs will enforce MAX_RELAY_DURATION.
    relayTasks[channel].offTime = millis() + duration;
}

// New function to trigger a relay disregarding the MAX_RELAY_DURATION clamp.
void triggerRelayLong(int channel, int duration)
{
    if (channel < 0 || channel >= NUM_RELAYS)
        return;
    sr.set(channel, HIGH);
    relayTasks[channel].active = true;
    relayTasks[channel].longActive = true;
    relayTasks[channel].startTime = millis();
    relayTasks[channel].offTime = millis() + duration;
}

// New function to disable a relay.
void disableRelay(int channel)
{
    if (channel < 0 || channel >= NUM_RELAYS)
        return;
    sr.set(channel, LOW);
    relayTasks[channel].active = false;
}

#define NUM_LEDS_FOR_TEST 4   // Number of LEDs for testing
CRGB leds[NUM_LEDS_FOR_TEST]; // LED array for SM16703

// Global variable definitions
int currentLitButton = 0;
int currentLastPressedButton = 0; // added definition for msg.lastPressedButton used in animations

void setup()
{
    // Use configuration values for reset.
    pinMode(RESET_PIN, OUTPUT);
    digitalWrite(RESET_PIN, LOW);  // Set LOW to trigger reset
    delay(RESET_LOW_DELAY_MS);     // Wait for reset to take effect
    digitalWrite(RESET_PIN, HIGH); // Set HIGH to end reset

    delay(SERIAL_MONITOR_DELAY_MS); // Wait for the serial monitor to open

    Serial.begin(921600); // Updated baud rate to match monitor_speed
    initSafeSerial();     // Initialize safe serial printing (new)

    NovaLogo();

    safeSerialPrintf("NOVA: MINI\n");
    safeSerialPrintf("setup() is running on core %d\n", xPortGetCoreID());

    Serial.setDebugOutput(true);

    safeSerialPrintf("Power on and set the 74HC595 to output disable mode\n");
    pinMode(HT74HC595_OUT_EN, OUTPUT);
    digitalWrite(HT74HC595_OUT_EN, HIGH); // Turn off output data HT74HC595

    safeSerialPrintf("Set all relay outputs to low level\n");
    // HT74HC595->setAllLow();
    sr.setAllLow();

    safeSerialPrintf("Set GPIO4 to low level to enable relay output\n");
    digitalWrite(HT74HC595_OUT_EN, LOW);

    // Initialize LittleFS and try to recover if mount fails.
    if (!LittleFS.begin())
    {
        safeSerialPrintf("LittleFS mount failed, formatting...\n");
        LittleFS.format();
        if (!LittleFS.begin())
        {
            safeSerialPrintf("LittleFS mount failed after format!\n");
        }
        else
        {
            safeSerialPrintf("LittleFS mount succeeded after format.\n");
        }
    }
    else
    {
        safeSerialPrintf("LittleFS mounted successfully.\n");
    }

    String macAddress = WiFi.macAddress();
    String AP_String = "";

    for (int i = 0; i < 6; i++)
    {
        uint8_t byteValue = strtoul(macAddress.substring(i * 3, i * 3 + 2).c_str(), NULL, 16);
        AP_String += String(byteValue, HEX);
    }

    AP_String = "NOVAMINI_" + AP_String.substring(0, 4);
    // your other setup stuff...
    WiFi.mode(WIFI_AP_STA); // Ensure WiFi mode is set to WIFI_AP_STA
    WiFi.softAP(AP_String, "scubadandy");
    WiFi.setSleep(false); // Disable power saving on the wifi interface.

    dnsServer.start(53, "*", WiFi.softAPIP());

    safeSerialPrintf("Setting up Webserver\n");
    webSetup();
    safeSerialPrintf("Setting up Webserver - Done\n");

    safeSerialPrintf("Create TaskWeb\n");
    delay(10);
    xTaskCreate(&TaskWeb, "TaskWeb", 8 * 1024, NULL, 5, NULL);
    delay(10);
    safeSerialPrintf("Create TaskWeb - Done\n");
    delay(10);

    safeSerialPrintf("Create TaskOutputs\n");
    delay(10);
    xTaskCreate(&TaskOutputs, "TaskOutputs", 8 * 1024, NULL, 5, NULL); // Now starting TaskOutputs
    delay(10);
    safeSerialPrintf("Create TaskOutputs - Done\n");

    // Create new TaskPulseRelay to handle relay #8 pulsing.
    safeSerialPrintf("Create TaskPulseRelay\n");
    delay(10);
    xTaskCreate(&TaskPulseRelay, "TaskPulseRelay", 4 * 1024, NULL, 5, NULL);
    delay(10);
    safeSerialPrintf("Create TaskPulseRelay - Done\n");

    // Call initEspNowReceiver to initialize receiver functionality.
    initEspNowReceiver();

    // Initialize FastLED for SM16703 LED support
    FastLED.addLeds<SM16703, LED_PIN_FASTLED, RGB>(leds, NUM_LEDS_FOR_TEST);

    // Create new TaskFastLED for SM16703 control
    safeSerialPrintf("Create TaskFastLED\n");
    xTaskCreate(&TaskFastLED, "TaskFastLED", 4 * 1024, NULL, 5, NULL);

    // New: Print device information
    Serial.println("Device Information:");
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    // Added new stats
    Serial.print("CPU Frequency (MHz): ");
    Serial.println(ESP.getCpuFreqMHz());
    Serial.print("SDK Version: ");
    Serial.println(ESP.getSdkVersion());
}

// Removed unused controlRelay function

void loop()
{
    // Do nothing in here. Everything must be in Tasks.
}

// Removed task definitions (TaskWeb, TaskOutputs, TaskPulseRelay, TaskFastLED)
// They have been moved to Tasks.cpp
