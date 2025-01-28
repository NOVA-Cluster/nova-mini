#include "Web.h"
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <WiFi.h>
#include "main.h"
#include <ESPUI.h>
#include <Arduino.h>

void handleRequest(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("<!DOCTYPE html><html><head><title>Captive Portal - NOVA</title></head><body>");
    response->print("<p>NOVA</p>");
    response->print("<p>This is our captive portal front page.</p>");
    response->printf("<p>You were trying to reach: http://%s%s</p>", request->host().c_str(), request->url().c_str());
    response->printf("<p>Try opening <a href='http://%s'>this link</a> instead</p>", WiFi.softAPIP().toString().c_str());
    response->print("</body></html>");
    request->send(response);
}

uint16_t switchOne;
uint16_t status;
uint16_t controlMillis;

uint16_t lightingBrightnessSlider, lightingSinSlider, lightingProgramSelect, lightingUpdatesSlider, lightingReverseSwitch, lightingFireSwitch, lightingLocalDisable, lightingAuto, lightingAutoTime;
uint16_t mainDrunktardSwitch;
uint16_t resetConfigSwitch, resetRebootSwitch;

uint16_t pooferA1, pooferA2, pooferA3, pooferA4;

uint16_t starManualPoof, starManualBlow, starManuallowFuel, starManualFuel, starManualZap, starManualSelect;
uint8_t starManualSelectValue = 0;

uint16_t sysInfoSeqIndex;

uint16_t seqBoomAll, seqBoomLeftRight, seqBoomRightLeft;

uint16_t starSeq_SEQ_POOF_END_TO_END, starSeq_SEQ_BOOMER_LEFT_TO_RIGHT, starSeq_SEQ_BOOMER_RIGHT_TO_LEFT, starSeq_SEQ_BOOM_FAST, starSeq_SEQ_BOOM_WAVE_IN, starSeq_SEQ_OFF;
uint16_t starSeq_SEQ_BOOM_POOF;


void numberCall(Control *sender, int type)
{
    Serial.println(sender->value);
}

void textCall(Control *sender, int type)
{
    Serial.print("Text: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}

void slider(Control *sender, int type)
{
    Serial.print("Slider: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}

void buttonCallback(Control *sender, int type)
{
    Serial.print("Button: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}

void switchExample(Control *sender, int value)
{

    Serial.print("Switch: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}

void selectExample(Control *sender, int value)
{
    Serial.print("Select: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}

void webSetup()
{
    Serial.println("In webSetup()");
    // Add tabs
    uint16_t mainTab = ESPUI.addControl(ControlType::Tab, "Main", "Main");
    // uint16_t settingsTab = ESPUI.addControl(ControlType::Tab, "Settings", "Settings");
    uint16_t manualTab = ESPUI.addControl(ControlType::Tab, "Manual", "Manual");
    uint16_t simonasTab = ESPUI.addControl(ControlType::Tab, "Simona", "Simona");
    uint16_t lightingTab = ESPUI.addControl(ControlType::Tab, "Lighting", "Lighting");
    uint16_t sysInfoTab = ESPUI.addControl(ControlType::Tab, "System Info", "System Info");
    uint16_t resetTab = ESPUI.addControl(ControlType::Tab, "Reset", "Reset");

    // Add status label above all tabs
    status = ESPUI.addControl(ControlType::Label, "Status:", "Unknown Status", ControlColor::Turquoise);

    //----- (Main) -----
    controlMillis = ESPUI.addControl(ControlType::Label, "Uptime", "0", ControlColor::Emerald, mainTab);
    // mainDrunktardSwitch = ESPUI.addControl(ControlType::Switcher, "Drunktard", String(enable->isDrunktard()), ControlColor::None, mainTab, &switchExample);

    //----- (Settings) -----
    // ESPUI.addControl(ControlType::Switcher, "Sleep (Disable)", "", ControlColor::None, settingsTab, &switchExample);

    //----- (Manual) -----

    pooferA1 = ESPUI.addControl(ControlType::Button, "Poofers", "Poof 1", ControlColor::Alizarin, manualTab, buttonCallback);
    pooferA2 = ESPUI.addControl(ControlType::Button, "", "Poof 2", ControlColor::None, pooferA1, buttonCallback);
    pooferA3 = ESPUI.addControl(ControlType::Button, "", "Poof 3", ControlColor::None, pooferA1, buttonCallback);
    pooferA4 = ESPUI.addControl(ControlType::Button, "", "Poof 4", ControlColor::None, pooferA1, buttonCallback);

    //---- Tab -- Lighting

    // System Info Tab
    sysInfoSeqIndex = ESPUI.addControl(ControlType::Label, "Button Sequence Index", "Red: 0, Green: 0, Blue: 0, Yellow: 0", ControlColor::Sunflower, sysInfoTab);

    // Reset tab
    ESPUI.addControl(ControlType::Label, "**WARNING**", "Don't even think of doing anything in this tab unless you want to break something!!", ControlColor::Sunflower, resetTab);
    resetConfigSwitch = ESPUI.addControl(ControlType::Switcher, "Reset Configurations", "0", ControlColor::Sunflower, resetTab, &switchExample);
    resetRebootSwitch = ESPUI.addControl(ControlType::Switcher, "Reboot", "0", ControlColor::Sunflower, resetTab, &switchExample);

    // Enable this option if you want sliders to be continuous (update during move) and not discrete (update on stop)
    // ESPUI.sliderContinuous = true;

    // Optionally use HTTP BasicAuth
    // ESPUI.server->addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP
    // ESPUI->server->begin();

    ESPUI.captivePortal = true;

    ESPUI.list(); // List all files on LittleFS, for info
    ESPUI.begin("NOVA Mini");
    Serial.println("Leaving webSetup()");
}

/**
 * Updates the web interface controls every n-second.
 */
void webLoop()
{
    // Initialize static variables
    static long oldTime = 0;
    static bool switchState = false;

    unsigned long currentMillis = millis();
    unsigned long seconds = (currentMillis / 1000) % 60;
    unsigned long minutes = (currentMillis / (1000 * 60)) % 60;
    unsigned long hours = (currentMillis / (1000 * 60 * 60)) % 24;
    unsigned long days = (currentMillis / (1000 * 60 * 60 * 24));

    String formattedTime = String(days) + "d " + String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";

    // Update controls every second
    if (millis() - oldTime > 1000)
    {
        // Toggle switch state
        switchState = !switchState;

        // Update switch and millis controls
        // ESPUI.updateControlValue(switchOne, switchState ? "1" : "0");
        ESPUI.updateControlValue(controlMillis, formattedTime);

        // Update oldTime
        oldTime = millis();

        /*
                if (enable->isSystemEnabled())
                {
                    if (enable->isDrunktard())
                    {
                        ESPUI.updateControlValue(status, "Drunktard");
                    }
                    else
                    {
                        ESPUI.updateControlValue(status, "Enabled");
                    }
                }
                else
                {
                    if (enable->isDrunktard())
                    {
                        ESPUI.updateControlValue(status, "System Disabled (Drunktard)");
                    }
                    else
                    {
                        ESPUI.updateControlValue(status, "System Disabled (Emergency Stop)");
                    }
                }

                String sequenceString = "Red: " + String(star->sequenceRed) + ", Green: " + String(star->sequenceGreen) + "<br>Blue: " + String(star->sequenceBlue) + ", Yellow: " + String(star->sequenceYellow);
                ESPUI.updateControlValue(sysInfoSeqIndex, sequenceString);
        */
    }
}