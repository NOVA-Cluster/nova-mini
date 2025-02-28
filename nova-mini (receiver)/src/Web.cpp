#include "Web.h"
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <WiFi.h>
#include "main.h"
#include <ESPUI.h>
#include <Arduino.h>
#include <Preferences.h> // Add this include

// Add new control IDs
uint16_t localMacLabel;
uint16_t connectedRemotesLabel;
uint16_t remoteMacText;
uint16_t savedRemoteMacLabel;
String remoteMacAddress = "";

// Add MAC address validation helper
bool isValidMacAddress(const String &mac)
{
    if (mac.length() != 17)
        return false;
    for (int i = 0; i < 17; i++)
    {
        if (i % 3 == 2)
        {
            if (mac.charAt(i) != ':')
                return false;
        }
        else
        {
            if (!isxdigit(mac.charAt(i)))
                return false;
        }
    }
    return true;
}

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
uint16_t mainDrunktardSwitch;
uint16_t pooferA1, pooferA2, pooferA3, pooferA4;
uint16_t dumpSwitch;
uint16_t dumpDurationSlider;

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
    if (type == B_DOWN)
    {
        // Handle the button down event
        Serial.print("Button Down: ID: ");
        Serial.print(sender->id);
        Serial.print(", Value: ");
        Serial.println(sender->value);

        // Map each poofers button to its relay channel with updated duration (250ms).
        if (sender->id == pooferA1)
        {
            triggerRelay(0, 250);
        }
        else if (sender->id == pooferA2)
        {
            triggerRelay(1, 250);
        }
        else if (sender->id == pooferA3)
        {
            triggerRelay(2, 250);
        }
        else if (sender->id == pooferA4)
        {
            triggerRelay(3, 250);
        }
    }
    else if (type == B_UP)
    {
        // Handle the button up event (do nothing)
    }
}

// Renamed callback function; previously "switchExample"
void switchCallback(Control *sender, int value)
{
    Serial.print("Switch: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);

    // If this is the Dump switch:
    if (sender->id == dumpSwitch)
    {
        if (sender->value == "1")
        {
            Serial.println("Dump switch activated: triggering pooferA1 for 10 seconds");
            triggerRelayLong(0, 10000);
        }
        else if (sender->value == "0")
        {
            Serial.println("Dump switch deactivated: turning off pooferA1");
            disableRelay(0);
        }
    }
    // ...existing behavior...
}

void selectExample(Control *sender, int value)
{
    Serial.print("Select: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}

// Add textCallback function for MAC address input
void textCallback(Control *sender, int type)
{
    if (sender->id == remoteMacText)
    {
        String newMac = sender->value;
        newMac.toUpperCase();
        ESPUI.updateControlValue(remoteMacText, newMac);

        if (newMac.isEmpty())
        {
            ESPUI.updateControlValue(status, "Enter remote's MAC address");
            return;
        }

        bool isValid = isValidMacAddress(newMac);
        Control *statusControl = ESPUI.getControl(status);
        if (!statusControl)
            return;

        if (isValid)
        {
            if (type == 10)
            { // Enter pressed or focus lost
                remoteMacAddress = newMac;
                Preferences preferences;
                preferences.begin("nova", false);
                preferences.putString("remote_mac", remoteMacAddress);
                preferences.end();

                ESPUI.updateControlValue(status, "✓ MAC address saved: " + remoteMacAddress);
                statusControl->color = ControlColor::Emerald;
                ESPUI.updateControl(statusControl);
                ESPUI.updateControlValue(savedRemoteMacLabel, remoteMacAddress);
                ESPUI.updateControlValue(remoteMacText, "");

                // Reinitialize ESP-NOW with new MAC
                initEspNowReceiver();
            }
            else
            {
                ESPUI.updateControlValue(status, "✓ Valid format - Press Enter to save");
                statusControl->color = ControlColor::Wetasphalt;
                ESPUI.updateControl(statusControl);
            }
        }
        else
        {
            String errorMsg = (newMac.length() != 17) ? "Length: " + String(newMac.length()) + "/17 chars" : "Invalid format. Use XX:XX:XX:XX:XX:XX";
            ESPUI.updateControlValue(status, "❌ " + errorMsg);
            statusControl->color = ControlColor::Alizarin;
            ESPUI.updateControl(statusControl);
        }
    }
}

void webSetup()
{
    Serial.println("In webSetup()");
    // Add tabs
    uint16_t mainTab = ESPUI.addControl(ControlType::Tab, "Main", "Main");
    uint16_t manualTab = ESPUI.addControl(ControlType::Tab, "Manual", "Manual");
    uint16_t dumpTab = ESPUI.addControl(ControlType::Tab, "Dump", "Dump");
    uint16_t espNowTab = ESPUI.addControl(ControlType::Tab, "ESPNow", "ESPNow");

    // Add Device Info label to Main tab.
    String deviceInfo = "MAC: " + WiFi.macAddress() + ", IP: " + WiFi.softAPIP().toString();
    uint16_t deviceInfoLabel = ESPUI.addControl(ControlType::Label, "Device Info", deviceInfo, ControlColor::None, mainTab);

    // Add status label above all tabs
    status = ESPUI.addControl(ControlType::Label, "Status:", "Unknown Status", ControlColor::Turquoise);

    //----- (Main) -----
    controlMillis = ESPUI.addControl(ControlType::Label, "Uptime", "0", ControlColor::Emerald, mainTab);

    //----- (Manual) -----
    pooferA1 = ESPUI.addControl(ControlType::Button, "Poofers", "Poof 1", ControlColor::Alizarin, manualTab, buttonCallback);
    pooferA2 = ESPUI.addControl(ControlType::Button, "", "Poof 2", ControlColor::None, pooferA1, buttonCallback);
    pooferA3 = ESPUI.addControl(ControlType::Button, "", "Poof 3", ControlColor::None, pooferA1, buttonCallback);
    pooferA4 = ESPUI.addControl(ControlType::Button, "", "Poof 4", ControlColor::None, pooferA1, buttonCallback);

    // Dump tab controls
    dumpDurationSlider = ESPUI.addControl(ControlType::Slider, "Dump Duration (secs)", "60", ControlColor::None, dumpTab, &slider);
    dumpSwitch = ESPUI.addControl(ControlType::Switcher, "Dump", "0", ControlColor::Alizarin, dumpTab, &switchCallback);

    // Add ESP-NOW tab controls
    ESPUI.addControl(
        ControlType::Label,
        "Instructions",
        "ESP-NOW Configuration:<br><br>"
        "1. This device's MAC address is shown below for remote devices to connect to<br>"
        "2. Enter the remote device's MAC address to establish the connection<br>"
        "3. The MAC format should be: XX:XX:XX:XX:XX:XX (uppercase)<br>"
        "4. Press Enter to save the MAC address",
        ControlColor::Carrot,
        espNowTab);

    String localMac = WiFi.macAddress();
    localMacLabel = ESPUI.addControl(
        ControlType::Label,
        "Local MAC Address",
        localMac,
        ControlColor::Turquoise,
        espNowTab);

    // Load saved remote MAC
    Preferences preferences;
    preferences.begin("nova", false);
    remoteMacAddress = preferences.getString("remote_mac", "");
    preferences.end();

    savedRemoteMacLabel = ESPUI.addControl(
        ControlType::Label,
        "Saved Remote MAC",
        remoteMacAddress.isEmpty() ? "Not set" : remoteMacAddress,
        ControlColor::Peterriver,
        espNowTab);

    remoteMacText = ESPUI.addControl(
        ControlType::Text,
        "Set Remote MAC Address",
        "",
        ControlColor::Alizarin,
        espNowTab,
        &textCallback);

    connectedRemotesLabel = ESPUI.addControl(
        ControlType::Label,
        "Connected Remotes",
        "No remotes connected",
        ControlColor::Peterriver,
        espNowTab);

    ESPUI.captivePortal = true;
    ESPUI.list();
    ESPUI.begin("NOVA Mini");
    Serial.println("Leaving webSetup()");
}

/**
 * Updates the web interface controls every n-second.
 */
void webLoop()
{
    static unsigned long lastUpdate = 0;
    unsigned long currentMillis = millis();

    // Update every 1000ms (1 second)
    if (currentMillis - lastUpdate >= 1000)
    {
        // Calculate time components
        unsigned long totalSeconds = currentMillis / 1000;
        unsigned long seconds = totalSeconds % 60;
        unsigned long minutes = (totalSeconds / 60) % 60;
        unsigned long hours = (totalSeconds / 3600) % 24;
        unsigned long days = totalSeconds / 86400;

        // Format time string
        String formattedTime = String(days) + "d " +
                               String(hours) + "h " +
                               String(minutes) + "m " +
                               String(seconds) + "s";

        // Update the display
        ESPUI.updateControlValue(controlMillis, formattedTime);

        lastUpdate = currentMillis;
    }

    // ...rest of existing webLoop code...
}