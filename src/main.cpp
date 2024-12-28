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

DNSServer dnsServer;
AsyncWebServer webServer(80);

void setup()
{
    Serial.begin(115200);
    Serial.println("NOVA: MINI");
    Serial.print("setup() is running on core ");
    Serial.println(xPortGetCoreID());

    Serial.setDebugOutput(true);

    Serial.println("Power on and set the 74HC595 to output disable mode");
    pinMode(HT74HC595_OUT_EN, OUTPUT);
    digitalWrite(HT74HC595_OUT_EN, HIGH); // Turn off output data HT74HC595

    Serial.println("Set all relay outputs to low level");
    //HT74HC595->setAllLow();
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
}

void loop()
{
    Serial.printf("System running time: %d\n\n", (uint32_t)millis() / 1000);

    /*
        Serial.println("All Channel turn on.");
        HT74HC595->setAllHigh();
        delay(2000);
        Serial.println("All Channel turn off.");
        HT74HC595->setAllLow();
        delay(2000);
     */

    Serial.println("Relay(CH1) turn on");
    sr.set(0, HIGH);
    delay(10000);
    Serial.println("Relay(CH2) turn on");
    sr.set(0, LOW);
    sr.set(1, HIGH);
    delay(10000);
    Serial.println("Relay(CH3) turn on");
    sr.set(1, LOW);
    sr.set(2, HIGH);
    delay(10000);
    Serial.println("Relay(CH4) turn on");
    sr.set(2, LOW);
    sr.set(3, HIGH);
    delay(10000);
    Serial.println("Relay(CH5) turn on");
    sr.set(3, LOW);
    sr.set(4, HIGH);
    delay(10000);
    Serial.println("Relay(CH6) turn on");
    sr.set(4, LOW);
    sr.set(5, HIGH);
    delay(10000);
    Serial.println("LED (CH7) turn on");
    sr.set(5, LOW);
    sr.set(6, HIGH);
    delay(10000);
    Serial.println("LED (CH8) turn on");
    sr.set(6, LOW);
    sr.set(7, HIGH);
    delay(10000);
}
