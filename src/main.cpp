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

/*
std::shared_ptr<ShiftRegister74HC595> HT74HC595 =
    std::make_shared<ShiftRegister74HC595>(8, HT74HC595_DATA,
                                                       HT74HC595_CLOCK, HT74HC595_LATCH);
*/

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

DNSServer dnsServer;
AsyncWebServer webServer(80);

void setup()
{
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

    Serial.println("Create TaskWeb");
    xTaskCreate(&TaskWeb, "TaskWeb", 16 * 1024, NULL, 5, NULL);
    Serial.println("Create TaskWeb - Done");
}

void controlRelay(int channel, const char *message, int delayTime)
{
    Serial.println(message);
    sr.set(channel, HIGH);
    delay(delayTime);
    sr.set(channel, LOW);
}

void loop()
{
    Serial.printf("System running time: %d\n\n", (uint32_t)millis() / 1000);

    controlRelay(0, "Relay(CH1) turn on", 10000);
    controlRelay(1, "Relay(CH2) turn on", 10000);
    controlRelay(2, "Relay(CH3) turn on", 10000);
    controlRelay(3, "Relay(CH4) turn on", 10000);
    controlRelay(4, "Relay(CH5) turn on", 10000);
    controlRelay(5, "Relay(CH6) turn on", 10000);
    controlRelay(6, "LED (CH7) turn on", 10000);
    controlRelay(7, "LED (CH8) turn on", 10000);
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
        // webLoop();

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