#include <Arduino.h>
#include "configuration.h" // New: Import configuration header
#include "Simona.h"
#include "utilities/utilities.h"  // Updated path
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "midi/MIDIControl.hpp"  // Updated path to MIDI module
#include <MIDI.h>
#include "EspNow.h"   // Now includes the declarations for espNowSetup() and espNowLoop()
#include <WiFi.h>     // Include WiFi library
#include <WiFiType.h> // Include WiFiType library
#include "Web.h"      // Include WebInterface header
#include "Tasks.h"    // Include Tasks.h
// Removed captive portal include
// #include "CaptivePortal.h" // <-- Removed

// Remove local pin definitions since they are now in configuration.h

uint8_t buttons[4] = {BTN_RED, BTN_GREEN, BTN_BLUE, BTN_YELLOW};
uint8_t leds[4] = {LED_RED, LED_GREEN, LED_BLUE, LED_YELLOW};
const char *buttonColors[4] = {"RED", "GREEN", "BLUE", "YELLOW"};
const char *ledColors[4] = {"RED", "GREEN", "BLUE", "YELLOW"};

boolean button[4] = {0, 0, 0, 0};

void WiFiEventHandler(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    Serial.println("Station Mode Started");
    break;
  case SYSTEM_EVENT_AP_START:
    Serial.println("AP Mode Started");
    break;
  case SYSTEM_EVENT_STA_CONNECTED:
    Serial.println("Connected to AP");
    break;
  case SYSTEM_EVENT_AP_STACONNECTED:
    Serial.println("Station connected to ESP32 AP");
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("Disconnected from AP");
    break;
  case SYSTEM_EVENT_AP_STADISCONNECTED:
    Serial.println("Station disconnected from ESP32 AP");
    break;
  }
}

void checkWiFiStatus(void *parameter)
{
  while (true)
  {
    Serial.printf("AP Stations connected: %d\n", WiFi.softAPgetStationNum());
    Serial.printf("WiFi Status: %s\n", WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
    vTaskDelay(pdMS_TO_TICKS(30000)); // Check every 30 seconds
  }
}

// Function to initialize LED with PWM
void initLedPWM(uint8_t pin, uint8_t channel) {
  ledcSetup(channel, LEDC_FREQ_HZ, LEDC_RESOLUTION);
  ledcAttachPin(pin, channel);
  ledcWrite(channel, LEDC_FULL_DUTY); // Initialize to full brightness (on state)
}

void setup()
{
  Serial.begin(115200); // Initialize serial communication for debugging
  delay(2000);          // Add delay to allow the serial monitor to connect

  // Initialize safe serial printing
  initSafeSerial();

  safeSerialPrintf("Initializing Nova Mini Remote\n");

  randomSeed(esp_random()); // Seed the random number generator with more entropy

  initializeMIDI();

  NovaLogo(); // Print the Nova logo

  // Initialize button pins
  pinMode(BTN_RED, INPUT);
  pinMode(BTN_GREEN, INPUT);
  pinMode(BTN_BLUE, INPUT);
  pinMode(BTN_YELLOW, INPUT);
  pinMode(BTN_RESET, INPUT);

  // Initialize LEDs with PWM
  initLedPWM(LED_RED, LEDC_CHANNEL_RED);
  initLedPWM(LED_GREEN, LEDC_CHANNEL_GREEN);
  initLedPWM(LED_BLUE, LEDC_CHANNEL_BLUE);
  initLedPWM(LED_YELLOW, LEDC_CHANNEL_YELLOW);
  initLedPWM(LED_RESET, LEDC_CHANNEL_RESET);

  if (0)
  {
    Serial.println("   Welcome To   ");
    Serial.println("> Simona <");
    Serial.println();
  }

  // Create FreeRTOS tasks

  WiFi.onEvent(WiFiEventHandler); // Register event handler

  // WiFi Setup
  WiFi.mode(WIFI_AP_STA); // Set WiFi to AP+STA mode
  WiFi.disconnect();

  // Create AP name with MAC suffix
  String apName = "NovaMiniRemote_" + getLastFourOfMac();
  WiFi.softAP(apName.c_str(), "scubadandy");

  // Print network information
  Serial.println("Device Information:");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("AP Name: ");
  Serial.println(apName);
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("STA IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Free Heap: ");
  Serial.println(ESP.getFreeHeap());
  Serial.print("CPU Frequency (MHz): ");
  Serial.println(ESP.getCpuFreqMHz());
  Serial.print("SDK Version: ");
  Serial.println(ESP.getSdkVersion());

  // Removed captive portal call:
  // startCaptivePortal();

  espNowSetup(); // call the renamed setup function from EspNow.cpp

  webSetup();

  // Initialize Simona singleton
  Simona::initInstance(buttons, leds, buttonColors, ledColors);

  // Create tasks (functions now defined in Tasks.cpp)
  xTaskCreate(gameTask, "Game Task", 4096, NULL, 1, NULL);
  xTaskCreate(buttonTask, "Button Task", 4096, NULL, 1, NULL);

  // Create WiFi monitoring task
  xTaskCreate(
      checkWiFiStatus,
      "WiFi Monitor",
      2048,
      NULL,
      1,
      NULL);

  // Create a new FreeRTOS task for the ESPUI web interface
  xTaskCreate(
      runEspuiTask,
      "ESPUI",
      8192, // Increased stack size for web interface
      NULL,
      1,
      NULL);

  xTaskCreate(
      runEspuiTask, // Function that implements the task
      "ESPUI_Task", // Text name for the task
      4096,         // Stack size in words
      NULL,         // Task input parameter
      1,            // Priority of the task
      NULL          // Task handle
  );

  playStartupMusic(); // New: play startup music (short, under 1.5 seconds)
}

void loop()
{
  // Removed captive portal DNS call:
  // dnsServer.processNextRequest();
  // Empty loop as tasks are handled by FreeRTOS
  espNowLoop(); // call the renamed loop function from EspNow.cpp
}

// Remove gameTask and buttonTask function definitions
