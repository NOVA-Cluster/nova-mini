#ifndef WEB_H
#define WEB_H

#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <ESPUI.h>

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

private:

};

extern uint16_t localMacLabel;
extern uint16_t connectedRemotesLabel;
extern uint16_t remoteMacText;
extern uint16_t savedRemoteMacLabel;
extern String remoteMacAddress;

void webSetup(void);

void webLoop(void);

#endif