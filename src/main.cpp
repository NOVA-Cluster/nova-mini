#undef SMOOTH_FONT

#include <ShiftRegister74HC595.h>
#include "pin_config.h"
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
#include "main.h" // Added include for initEspNowReceiver and relay control prototypes
#include "Utilities.h"  // Added include for safeSerialPrintf and initialization

#include "Web.h"
#include <stdarg.h>
#include "freertos/semphr.h"

// Define the number of 74HC595 chips in cascade
const uint8_t NUMBER_OF_SHIFT_REGISTERS = 1;

// Create a ShiftRegister74HC595 object
ShiftRegister74HC595<NUMBER_OF_SHIFT_REGISTERS> sr(HT74HC595_DATA, HT74HC595_CLOCK, HT74HC595_LATCH); // (dataPin, clockPin, latchPin)

void TaskWeb(void *pvParameters);
void TaskOutputs(void *pvParameters);
void TaskPulseRelay(void *pvParameters);

DNSServer dnsServer;
AsyncWebServer webServer(80);

// Configurable maximum relay duration in ms
const uint32_t MAX_RELAY_DURATION = 250;

// Updated non-blocking relay control implementation
struct RelayTask
{
    bool active;
    bool longActive; // New flag for long duration events.
    uint32_t offTime;
    uint32_t startTime; // New field to record when the relay was turned on
};
const int NUM_RELAYS = 8;
RelayTask relayTasks[NUM_RELAYS] = {{false, false, 0, 0}};

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

void setup()
{
    pinMode(39, OUTPUT);
    digitalWrite(39, LOW);  // Set LOW to trigger reset
    delay(50);              // Wait for reset to take effect
    digitalWrite(39, HIGH); // Set HIGH to end reset

    delay(3000); // Wait for the serial monitor to open

    Serial.begin(921600); // Updated baud rate to match monitor_speed
    initSafeSerial(); // Initialize safe serial printing (new)

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
    if(!LittleFS.begin()){
        safeSerialPrintf("LittleFS mount failed, formatting...\n");
        LittleFS.format();
        if(!LittleFS.begin()){
            safeSerialPrintf("LittleFS mount failed after format!\n");
        } else {
            safeSerialPrintf("LittleFS mount succeeded after format.\n");
        }
    } else {
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
    WiFi.softAP(AP_String, "scubadandy");
    WiFi.setSleep(false); // Disable power saving on the wifi interface.

    dnsServer.start(53, "*", WiFi.softAPIP());

    safeSerialPrintf("Setting up Webserver\n");
    webSetup();
    safeSerialPrintf("Setting up Webserver - Done\n");

    // Call initEspNowReceiver to initialize receiver functionality.
    initEspNowReceiver();

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
}

// Removed unused controlRelay function

void loop()
{
    // Do nothing in here. Everything must be in Tasks.
}

void TaskWeb(void *pvParameters) // This is a task.
{
    (void)pvParameters;
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);

    safeSerialPrintf("TaskWeb is running\n");
    while (1) // A Task shall never return or exit.
    {
        webLoop();

        yield(); // Should't do anything but it's here incase the watchdog needs it.
        delay(1);

        static uint32_t lastExecutionTime = 0;
        if (millis() - lastExecutionTime >= REPORT_TASK_INTERVAL)
        {
            /* Calling the function will have used some stack space, we would
                therefore now expect uxTaskGetStackHighWaterMark() to return a
                value lower than when it was called on entering the task. */
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = millis();
        }
    }
}

// Updated TaskOutputs using non blocking delays.
void TaskOutputs(void *pvParameters) // This is a task.
{
    (void)pvParameters;
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);

    safeSerialPrintf("TaskOutputs is running\n");
    while (1) // A Task shall never return or exit.
    {
        uint32_t currentTime = millis();
        // Turn off any relays whose duration has elapsed or exceed maximum allowed time.
        for (int i = 0; i < NUM_RELAYS; i++)
        {
            if (relayTasks[i].active)
            {
                // For longActive relays, only offTime matters.
                if (relayTasks[i].longActive)
                {
                    if (currentTime >= relayTasks[i].offTime)
                    {
                        sr.set(i, LOW);
                        relayTasks[i].active = false;
                    }
                }
                else
                {
                    // For normal pulses, enforce MAX_RELAY_DURATION.
                    if (currentTime >= relayTasks[i].offTime ||
                        (currentTime - relayTasks[i].startTime) >= MAX_RELAY_DURATION)
                    {
                        sr.set(i, LOW);
                        relayTasks[i].active = false;
                    }
                }
            }
        }

        // ... Code to process incoming relay messages ...
        // For example, to simulate turning on relay 1 for 50 ms, uncomment:
        // triggerRelay(1, 50);

        yield(); // let other tasks run
        delay(1);

        static uint32_t lastExecutionTime = 0;
        if (millis() - lastExecutionTime >= REPORT_TASK_INTERVAL)
        {
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = millis();
        }
    }
}

// New task to pulse relay #8 every second.
void TaskPulseRelay(void *pvParameters)
{
    (void)pvParameters;
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);
    safeSerialPrintf("TaskPulseRelay is running\n");
    while(1)
    {
        uint32_t currentTime = millis();
        // Pulse relay #8 (index 7) on for 100 ms every second.
        static uint32_t lastPulse = 0;
        if(currentTime - lastPulse >= 1000)
        {
            triggerRelay(7, 100);
            lastPulse = currentTime;
        }
        yield();
        delay(1);
        static uint32_t lastExecutionTime = 0;
        if(currentTime - lastExecutionTime >= REPORT_TASK_INTERVAL)
        {
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = currentTime;
        }
    }
}
