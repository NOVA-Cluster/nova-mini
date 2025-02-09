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

#include "Web.h"

// Define the number of 74HC595 chips in cascade
const uint8_t NUMBER_OF_SHIFT_REGISTERS = 1;

// Create a ShiftRegister74HC595 object
ShiftRegister74HC595<NUMBER_OF_SHIFT_REGISTERS> sr(HT74HC595_DATA, HT74HC595_CLOCK, HT74HC595_LATCH); // (dataPin, clockPin, latchPin)

/*
std::shared_ptr<ShiftRegister74HC595_NonTemplate> HT74HC595 =
    std::make_shared<ShiftRegister74HC595_NonTemplate>(8, HT74HC595_DATA,
                                                       HT74HC595_CLOCK, HT74HC595_LATCH);
*/

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
    uint32_t offTime;
    uint32_t startTime; // New field to record when the relay was turned on
};
const int NUM_RELAYS = 8;
RelayTask relayTasks[NUM_RELAYS] = {{false, 0, 0}};

// Function to trigger a relay for a specified duration (in ms), clamped by MAX_RELAY_DURATION.
void triggerRelay(int channel, int duration)
{
    if (channel < 0 || channel >= NUM_RELAYS)
        return;
    sr.set(channel, HIGH);
    relayTasks[channel].active = true;
    relayTasks[channel].startTime = millis();
    // Use the requested duration but TaskOutputs will enforce MAX_RELAY_DURATION.
    relayTasks[channel].offTime = millis() + duration;
}

void setup()
{
    pinMode(39, OUTPUT);
    digitalWrite(39, LOW);  // Set high the rest pin
    delay(50);              // Wait for reset to take effect
    digitalWrite(39, HIGH); // Set high the rest pin

    delay(3000); // Wait for the serial monitor to open

    Serial.begin(115200);
    Serial.println("NOVA: MINI");
    Serial.print("setup() is running on core ");
    Serial.println(xPortGetCoreID());

    Serial.setDebugOutput(true);

    Serial.println("Power on and set the 74HC595 to output disable mode");
    pinMode(HT74HC595_OUT_EN, OUTPUT);
    digitalWrite(HT74HC595_OUT_EN, HIGH); // Turn off output data HT74HC595

    Serial.println("Set all relay outputs to low level");
    // HT74HC595->setAllLow();
    sr.setAllLow();

    Serial.println("Set GPIO4 to low level to enable relay output");
    digitalWrite(HT74HC595_OUT_EN, LOW);

    // Initialize LittleFS and try to recover if mount fails.
    if(!LittleFS.begin()){
        Serial.println("LittleFS mount failed, formatting...");
        LittleFS.format();
        if(!LittleFS.begin()){
            Serial.println("LittleFS mount failed after format!");
        } else {
            Serial.println("LittleFS mount succeeded after format.");
        }
    } else {
        Serial.println("LittleFS mounted successfully.");
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

    Serial.println("Setting up Webserver");
    webSetup();
    Serial.println("Setting up Webserver - Done");

    Serial.println("Create TaskWeb");
    delay(10);
    xTaskCreate(&TaskWeb, "TaskWeb", 8 * 1024, NULL, 5, NULL);
    delay(10);
    Serial.println("Create TaskWeb - Done");
    delay(10);

    Serial.println("Create TaskOutputs");
    delay(10);
    xTaskCreate(&TaskOutputs, "TaskOutputs", 8 * 1024, NULL, 5, NULL); // Now starting TaskOutputs
    delay(10);
    Serial.println("Create TaskOutputs - Done");

    // Create new TaskPulseRelay to handle relay #8 pulsing.
    Serial.println("Create TaskPulseRelay");
    delay(10);
    xTaskCreate(&TaskPulseRelay, "TaskPulseRelay", 4 * 1024, NULL, 5, NULL);
    delay(10);
    Serial.println("Create TaskPulseRelay - Done");
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

    Serial.println("TaskWeb is running");
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
            Serial.printf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
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

    Serial.println("TaskOutputs is running");
    while (1) // A Task shall never return or exit.
    {
        uint32_t currentTime = millis();
        // Turn off any relays whose duration has elapsed or exceed maximum allowed time.
        for (int i = 0; i < NUM_RELAYS; i++)
        {
            if (relayTasks[i].active && (currentTime >= relayTasks[i].offTime || (currentTime - relayTasks[i].startTime) >= MAX_RELAY_DURATION))
            {
                sr.set(i, LOW);
                relayTasks[i].active = false;
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
            Serial.printf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
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
    Serial.println("TaskPulseRelay is running");
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
            Serial.printf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = currentTime;
        }
    }
}
