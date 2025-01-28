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
#include <MIDI.h>
#include <SoftwareSerial.h>
#include "MIDIControl.hpp"

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
void TaskMidi(void *pvParameters);
void TaskOutputs(void *pvParameters);

DNSServer dnsServer;
AsyncWebServer webServer(80);

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

    initializeMIDI();

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

    Serial.println("Create TaskMidi");
    delay(10);
    // xTaskCreate(&TaskMidi, "TaskMidi", 8 * 1024, NULL, 100, NULL);
    delay(10);
    Serial.println("Create TaskMidi - Done");
    delay(10);

    Serial.println("Create TaskOutputs");
    delay(10);
    // xTaskCreate(&TaskOutputs, "TaskOutputs", 8 * 1024, NULL, 5, NULL);
    delay(10);
    Serial.println("Create TaskOutputs - Done");
}

void controlRelay(int channel, const char *message, int delayTime)
{
    // Serial.println(message);
    sr.set(channel, HIGH);
    delay(delayTime);
    sr.set(channel, LOW);
}

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

void TaskMidi(void *pvParameters) // This is a task.
{
    (void)pvParameters;
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);

    Serial.println("TaskMidi is running");
    while (1) // A Task shall never return or exit.
    {
        // Driving beat (Channel 10)
        playTechnoBeat();

        // Bassline (Channel 2)
        playTechnoBassline();

        // Minimal melody (Channel 1)
        playTechnoMelody();

        delay(2000); // Wait before repeating the sequence

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

void TaskOutputs(void *pvParameters) // This is a task.
{
    (void)pvParameters;
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);

    Serial.println("TaskOutputs is running");
    while (1) // A Task shall never return or exit.
    {
        controlRelay(0, "Relay(CH1) turn on", 1000);
        controlRelay(1, "Relay(CH2) turn on", 1000);
        controlRelay(2, "Relay(CH3) turn on", 1000);
        controlRelay(3, "Relay(CH4) turn on", 1000);
        controlRelay(4, "Relay(CH5) turn on", 1000);
        controlRelay(5, "Relay(CH6) turn on", 1000);
        controlRelay(6, "LED (CH7) turn on", 1000);
        controlRelay(7, "LED (CH8) turn on", 1000);

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
