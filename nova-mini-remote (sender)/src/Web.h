#ifndef WEB_H
#define WEB_H

#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <ESPUI.h>

// Add these constants for preference handling
#define PREF_NAMESPACE "nova"
#define PREF_CHEAT_MODE "cheat_mode"

// Runtime variables
extern bool SIMONA_CHEAT_MODE;  // Managed by web interface

// Add these new external variables
extern uint16_t receiverMacText;
extern uint16_t localMacLabel;
extern String receiverMacAddress;

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request)
  {
    // request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", 
        "<!DOCTYPE html><html><head>"
        "<title>Success</title>"
        "<style>body{text-align:center;font-family:helvetica,arial;}</style>"
        "</head><body>"
        "<h1>&#x2705; Success</h1>"
        "<p>You are connected!</p>"
        "</body></html>");
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
  }

private:

};

void webSetup(void);
void runEspuiTask(void *parameter);
void webLoop();
void runEspuiTask(void* pvParameters); // Add this line

extern uint16_t gameSpeedSlider;
extern uint16_t volumeSlider;
extern uint16_t statusText;
extern uint16_t scoreText;

extern DNSServer dnsServer;

#endif