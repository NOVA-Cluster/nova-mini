#include "Web.h"
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <WiFi.h>
#include "main.h"
#include <ESPUI.h>
#include <Arduino.h>
#include "Simona.h" // Add this include
#include "freertos/semphr.h"
#include <Preferences.h>
#include "utilities/PreferencesManager.h" // Updated path
#include "EspNow.h"                       // Add this include for packet loss stats
#include "EStop.h"        // Include EStop header for E-Stop functionality

DNSServer dnsServer;

// Add this line to define the external variable
bool SIMONA_CHEAT_MODE = false; // Define the external variable with default value
bool WIRELESS_ENABLED = true;   // Make sure this is defined at global scope
bool GAME_ENABLED = true;       // Define game enabled with default value true

SemaphoreHandle_t dnsMutex = NULL;
bool dnsServerActive = false;

Preferences preferences;
uint16_t settingsTab;
uint16_t espNowTab;
uint16_t cheatModeSwitch;
uint16_t wirelessEnabledSwitch; // Add new control ID
uint16_t gameEnabledSwitch;     // Add new control ID

// Add new control IDs
uint16_t receiverMacText;
uint16_t localMacLabel;
uint16_t savedReceiverMacLabel; // Add new control ID for saved MAC display
uint16_t factoryResetText;      // Add new control ID for factory reset
String receiverMacAddress = "";

// Add packet loss label control ID
uint16_t packetLossLabel;

// Add new control ID for sequence local echo
uint16_t sequenceLocalEchoSwitch;

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

void handleCaptivePortal(AsyncWebServerRequest *request)
{
    Serial.printf("Captive Portal Request - Host: %s, URL: %s\n", request->host().c_str(), request->url().c_str());

    // Check if it's an iOS device checking for captive portal
    if (request->host() == "captive.apple.com")
    {
        Serial.println("iOS captive portal check detected");
        AsyncWebServerResponse *response = request->beginResponse(200, "text/html",
                                                                  "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "-1");
        request->send(response);
        return;
    }

    // For all other requests, redirect to the main interface
    Serial.println("Redirecting to main interface");
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "");
    response->addHeader("Location", "http://" + WiFi.softAPIP().toString());
    request->send(response);
}

uint16_t switchOne;
uint16_t status;
uint16_t controlMillis;
uint16_t sysInfoSeqIndex;
uint16_t simonaProgressLabel;
uint16_t simonaNextColorLabel;
uint16_t simonaTimeRemainingLabel;

// Remove all other global control variables as they're no longer needed

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

    if (sender->id == cheatModeSwitch)
    {
        SIMONA_CHEAT_MODE = (sender->value == "1");
        PreferencesManager::setBool(PREF_KEY_CHEAT_MODE, SIMONA_CHEAT_MODE);
        Simona::getInstance()->setCheatMode(SIMONA_CHEAT_MODE); // Use singleton directly
        Serial.printf("Cheat mode %s and saved to preferences\n", SIMONA_CHEAT_MODE ? "enabled" : "disabled");
    }
    else if (sender->id == wirelessEnabledSwitch)
    {
        WIRELESS_ENABLED = (sender->value == "1");
        PreferencesManager::setBool(PREF_KEY_WIRELESS_ENABLED, WIRELESS_ENABLED);
        Serial.printf("Wireless %s and saved to preferences\n", WIRELESS_ENABLED ? "enabled" : "disabled");
    }
    else if (sender->id == gameEnabledSwitch)
    {
        GAME_ENABLED = (sender->value == "1");
        PreferencesManager::setBool(PREF_KEY_GAME_ENABLED, GAME_ENABLED);
        Serial.printf("Game %s and saved to preferences\n", GAME_ENABLED ? "enabled" : "disabled");
    }
    else if (sender->id == sequenceLocalEchoSwitch)
    {
        bool enabled = (sender->value == "1");
        PreferencesManager::setBool(PREF_KEY_SEQUENCE_LOCAL_ECHO, enabled);
        Simona::getInstance()->setSequenceLocalEcho(enabled);
        Serial.printf("Sequence local echo %s and saved to preferences\n", enabled ? "enabled" : "disabled");
    }
}

void selectExample(Control *sender, int value)
{
    Serial.print("Select: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}

// Add this helper function at the top with other utility functions
bool isValidMacAddress(const String &mac)
{
    if (mac.length() != 17)
        return false; // XX:XX:XX:XX:XX:XX = 17 chars

    // Check format: should be XX:XX:XX:XX:XX:XX where X is a hex digit
    for (int i = 0; i < 17; i++)
    {
        if (i % 3 == 2)
        {
            // Every third character should be a colon
            if (mac.charAt(i) != ':')
                return false;
        }
        else
        {
            // All other characters should be hex digits
            if (!isxdigit(mac.charAt(i)))
                return false;
        }
    }
    return true;
}

// Replace the existing textCallback function with this enhanced version
void textCallback(Control *sender, int type)
{
    Serial.printf("\n=== Text Callback ===\n");
    Serial.printf("ID: %d, Type: %d, Value: %s\n", sender->id, type, sender->value.c_str());

    if (sender->id == receiverMacText)
    {
        Serial.println("Handling MAC address input");

        // Debug print all event details
        Serial.printf("Event Type: %d\n", type);

        String newMac = sender->value;

        // Don't validate empty strings
        if (newMac.isEmpty())
        {
            Serial.println("Empty input - showing initial prompt");
            ESPUI.updateControlValue(status, "Enter MAC address");
            Control *statusControl = ESPUI.getControl(status);
            if (statusControl)
            {
                statusControl->color = ControlColor::Wetasphalt;
                ESPUI.updateControl(statusControl);
            }
            return;
        }

        // Convert to uppercase immediately
        newMac.toUpperCase();
        Serial.printf("Converted to uppercase: %s\n", newMac.c_str());

        // Always update the field to uppercase
        ESPUI.updateControlValue(receiverMacText, newMac);

        // Prepare validation feedback
        String errorMsg;
        bool isValid = isValidMacAddress(newMac);

        // Get status control
        Control *statusControl = ESPUI.getControl(status);
        if (!statusControl)
        {
            Serial.println("ERROR: Could not get status control!");
            return;
        }

        if (isValid)
        {
            Serial.println("MAC format is valid");

            // Save on either Enter key or losing focus (type 10)
            if (type == 10)
            {
                Serial.println("Saving MAC address (Enter pressed or focus lost)");
                receiverMacAddress = newMac;
                PreferencesManager::setString(PREF_KEY_RECEIVER_MAC, receiverMacAddress);

                ESPUI.updateControlValue(status, "✓ MAC address saved: " + receiverMacAddress);
                statusControl->color = ControlColor::Emerald;
                ESPUI.updateControl(statusControl);

                // Reinitialize ESP-NOW with new MAC
                // You might want to add a call to reinitialize ESP-NOW here
                Serial.println("MAC address saved to preferences: " + receiverMacAddress);

                // Update both status and saved MAC display
                ESPUI.updateControlValue(savedReceiverMacLabel, receiverMacAddress);

                // Clear the input field
                ESPUI.updateControlValue(receiverMacText, "");
            }
            else
            {
                // Valid but not saved yet
                ESPUI.updateControlValue(status, "✓ Valid format - Press Enter to save");
                statusControl->color = ControlColor::Wetasphalt;
                ESPUI.updateControl(statusControl);
            }
        }
        else
        {
            // Show validation error with specific feedback
            if (newMac.length() != 17)
            {
                errorMsg = "Length: " + String(newMac.length()) + "/17 chars";
            }
            else
            {
                errorMsg = "Invalid format. Use XX:XX:XX:XX:XX:XX";
            }
            Serial.println("Invalid MAC: " + errorMsg);
            ESPUI.updateControlValue(status, "❌ " + errorMsg);
            statusControl->color = ControlColor::Alizarin;
            ESPUI.updateControl(statusControl);
        }
    }
    else if (sender->id == factoryResetText)
    {
        // Get status control for feedback
        Control *statusControl = ESPUI.getControl(status);
        if (!statusControl)
        {
            Serial.println("ERROR: Could not get status control!");
            return;
        }

        // Process reset command
        if (sender->value == "RESET" && type == 10)
        { // Type 10 means Enter was pressed
            Serial.println("FACTORY RESET COMMAND RECEIVED");

            // Show status
            ESPUI.updateControlValue(status, "⚠️ Factory Reset in progress - device will reboot");
            statusControl->color = ControlColor::Alizarin;
            ESPUI.updateControl(statusControl);

            // Clear the input field
            ESPUI.updateControlValue(factoryResetText, "");

            // Give UI time to update
            delay(1000);

            // Clear all preferences
            Serial.println("Clearing all preferences");
            Preferences p;
            p.begin(PREF_NAMESPACE, false);
            p.clear();
            p.end();

            // Reboot
            Serial.println("Rebooting device");
            ESP.restart();
        }
        else if (!sender->value.isEmpty() && sender->value != "RESET")
        {
            // Invalid input
            ESPUI.updateControlValue(status, "❌ Only 'RESET' (all caps) is accepted");
            statusControl->color = ControlColor::Alizarin;
            ESPUI.updateControl(statusControl);
        }
        else if (sender->value.isEmpty())
        {
            // Empty field - no message
            ESPUI.updateControlValue(status, "Type 'RESET' to factory reset");
            statusControl->color = ControlColor::Wetasphalt;
            ESPUI.updateControl(statusControl);
        }
    }

    Serial.println("=== End Text Callback ===\n");
}

void tabCallback(Control *sender, int type)
{
    // Use sender->label instead of sender->id for string comparison
    if (strcmp(sender->label, "Cheat Mode") == 0)
    {
        SIMONA_CHEAT_MODE = !SIMONA_CHEAT_MODE;
        PreferencesManager::setBool(PREF_KEY_CHEAT_MODE, SIMONA_CHEAT_MODE);
        ESPUI.updateControlValue(sender, SIMONA_CHEAT_MODE ? "On" : "Off");
    }
}

void webSetup()
{
    Serial.println("In webSetup()");

    // Load saved preferences
    SIMONA_CHEAT_MODE = PreferencesManager::getBool(PREF_KEY_CHEAT_MODE);
    WIRELESS_ENABLED = PreferencesManager::getBool(PREF_KEY_WIRELESS_ENABLED, true); // Default to true
    GAME_ENABLED = PreferencesManager::getBool(PREF_KEY_GAME_ENABLED, true);         // Default to true
    receiverMacAddress = PreferencesManager::getString(PREF_KEY_RECEIVER_MAC);

    Serial.printf("Loaded cheat mode from preferences: %s\n", SIMONA_CHEAT_MODE ? "enabled" : "disabled");
    Serial.printf("Loaded wireless enabled from preferences: %s\n", WIRELESS_ENABLED ? "enabled" : "disabled");
    Serial.printf("Loaded game enabled from preferences: %s\n", GAME_ENABLED ? "enabled" : "disabled");

    // Create mutex for DNS operations
    dnsMutex = xSemaphoreCreateMutex();

    // Configure DNS server to respond to all domains
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServerActive = dnsServer.start(53, "*", WiFi.softAPIP());

    // Add all tabs
    uint16_t mainTab = ESPUI.addControl(ControlType::Tab, "Main", "Main");
    uint16_t sysInfoTab = ESPUI.addControl(ControlType::Tab, "System Info", "System Info");
    settingsTab = ESPUI.addControl(ControlType::Tab, "Settings", "Settings");
    espNowTab = ESPUI.addControl(ControlType::Tab, "ESPNow", "ESPNow");

    // Add Device Info label to System Info tab
    String deviceInfo = "MAC: " + WiFi.macAddress() + ", IP: " + WiFi.softAPIP().toString();
    uint16_t deviceInfoLabel = ESPUI.addControl(ControlType::Label, "Device Info", deviceInfo, ControlColor::None, sysInfoTab);

    // Move uptime to System Info tab (was previously in Main tab)
    controlMillis = ESPUI.addControl(ControlType::Label, "Uptime", "0", ControlColor::Emerald, sysInfoTab);

    // Add packet loss label to System Info tab
    packetLossLabel = ESPUI.addControl(
        ControlType::Label,
        "Packet Loss (5min)",
        "Calculating...",
        ControlColor::Peterriver,
        sysInfoTab);

    // Add status label above all tabs
    status = ESPUI.addControl(ControlType::Label, "Status:", "Unknown Status", ControlColor::Turquoise);

    //----- (Main) -----
    simonaProgressLabel = ESPUI.addControl(ControlType::Label, "Simona Progress", "Not started", ControlColor::Emerald, mainTab);
    simonaNextColorLabel = ESPUI.addControl(ControlType::Label, "Expected Color", "Waiting...", ControlColor::Emerald, mainTab);
    simonaTimeRemainingLabel = ESPUI.addControl(ControlType::Label, "Time Remaining", "-", ControlColor::Emerald, mainTab);

    //----- (Settings) -----
    cheatModeSwitch = ESPUI.addControl(ControlType::Switcher, "Cheat Mode",
                                       SIMONA_CHEAT_MODE ? "1" : "0",
                                       ControlColor::Carrot,
                                       settingsTab,
                                       &switchCallback);

    // Add wireless enabled toggle below cheat mode
    wirelessEnabledSwitch = ESPUI.addControl(ControlType::Switcher, "Wireless Enabled",
                                             WIRELESS_ENABLED ? "1" : "0",
                                             ControlColor::Peterriver,
                                             settingsTab,
                                             &switchCallback);

    // Add game enabled toggle below wireless enabled
    gameEnabledSwitch = ESPUI.addControl(ControlType::Switcher, "Game Enabled",
                                         GAME_ENABLED ? "1" : "0",
                                         ControlColor::Peterriver,
                                         settingsTab,
                                         &switchCallback);

    // Add sequence local echo toggle below game enabled
    bool sequenceLocalEcho = PreferencesManager::getBool(PREF_KEY_SEQUENCE_LOCAL_ECHO, true);
    sequenceLocalEchoSwitch = ESPUI.addControl(ControlType::Switcher, "Sequence Local Echo",
                                               sequenceLocalEcho ? "1" : "0",
                                               ControlColor::Peterriver,
                                               settingsTab,
                                               &switchCallback);

    // Add tooltip explaining sequence local echo
    ESPUI.addControl(ControlType::Label, "",
                     "When disabled, LED sequences will only play on NOVA, not on the remote",
                     ControlColor::None,
                     sequenceLocalEchoSwitch);

    // Remove the separator line - it was ugly

    // Add Factory Reset warning and instructions
    ESPUI.addControl(
        ControlType::Label,
        "Factory Reset",
        "⚠️ <strong>WARNING: Factory Reset</strong> ⚠️<br><br>"
        "To reset all settings to default values:<br>"
        "1. Type the word <strong>RESET</strong> (all caps) in the field below<br>"
        "2. Press Enter to confirm<br><br>"
        "This will erase all saved settings and reboot the device.<br>"
        "MAC addresses, wireless settings, and game preferences will be reset to defaults.",
        ControlColor::Alizarin,
        settingsTab);

    // Add the factory reset text field
    factoryResetText = ESPUI.addControl(
        ControlType::Text,
        "Enter 'RESET' to restore defaults",
        "", // Start with empty field
        ControlColor::Alizarin,
        settingsTab,
        &textCallback);

    //----- (ESPNow) -----
    // Load saved receiver MAC address
    receiverMacAddress = PreferencesManager::getString(PREF_KEY_RECEIVER_MAC);

    // Modify the ESPNow tab section
    ESPUI.addControl(
        ControlType::Label,
        "Instructions",
        "How to connect to your NOVA Mini:<br><br>"
        "1. On NOVA Mini (the receiver), look for a sticker with its MAC address<br>"
        "2. The MAC address format should look like: 40:91:51:XX:XX:XX<br>"
        "3. Enter the complete MAC address in the field below<br>"
        "4. Press Enter to save<br><br>"
        "Example MAC addresses:<br>"
        "40:91:51:A1:B2:C3<br>"
        "40:91:51:12:34:56<br><br>"
        "Note: All characters must be in uppercase, and colons (:) are required.",
        ControlColor::Carrot,
        espNowTab);

    String localMac = WiFi.macAddress();
    localMacLabel = ESPUI.addControl(
        ControlType::Label,
        "Local MAC Address",
        localMac,
        ControlColor::Turquoise,
        espNowTab);

    // Add label to show currently saved MAC
    savedReceiverMacLabel = ESPUI.addControl(
        ControlType::Label,
        "Saved Receiver MAC Address",
        receiverMacAddress.isEmpty() ? "Not set" : receiverMacAddress,
        ControlColor::Peterriver,
        espNowTab);

    receiverMacText = ESPUI.addControl(
        ControlType::Text,
        "Set Receiver MAC Address",
        "", // Start with empty field
        ControlColor::Alizarin,
        espNowTab,
        &textCallback);

    ESPUI.captivePortal = true;

    // Ensure the captive portal handler is registered
    AsyncWebServer *server = ESPUI.WebServer();
    if (server)
    {
        server->onNotFound([](AsyncWebServerRequest *request)
                           { handleCaptivePortal(request); });
    }

    ESPUI.begin("NOVA Mini Remote");

    Serial.println("Leaving webSetup()");
}

void safeDnsStop()
{
    if (dnsMutex != NULL && xSemaphoreTake(dnsMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        if (dnsServerActive)
        {
            dnsServer.stop();
            dnsServerActive = false;
        }
        xSemaphoreGive(dnsMutex);
    }
}

/**
 * Updates the web interface controls every n-second.
 */
ControlColor getColorForName(const char *colorName)
{
    if (!colorName)
        return ControlColor::Dark;

    String color = String(colorName);
    color.toLowerCase();

    if (color == "red")
        return ControlColor::Alizarin; // Red
    if (color == "green")
        return ControlColor::Wetasphalt; // Changed to a different color for green
    if (color == "blue")
        return ControlColor::Peterriver; // Blue
    if (color == "yellow")
        return ControlColor::Sunflower; // Yellow

    return ControlColor::Dark;
}

void webLoop()
{
    // Initialize static variables
    static long oldTime = 0;
    static long lastDnsCheck = 0;
    static bool switchState = false;
    static long lastPacketLossUpdate = 0;
    static long lastStatusUpdate = 0; // New variable for status updates

    // Process DNS requests at most every 250ms with mutex protection
    if (millis() - lastDnsCheck >= 250)
    {
        if (dnsMutex != NULL && xSemaphoreTake(dnsMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            if (dnsServerActive)
            {
                try
                {
                    dnsServer.processNextRequest();
                }
                catch (...)
                {
                    // On error, attempt to restart DNS server
                    dnsServer.stop();
                    dnsServerActive = dnsServer.start(53, "*", WiFi.softAPIP());
                }
            }
            xSemaphoreGive(dnsMutex);
        }
        lastDnsCheck = millis();
    }

    // Update system status message every second
    if (millis() - lastStatusUpdate >= 1000)
    {
        Control *statusControl = ESPUI.getControl(status);
        if (statusControl)
        {
            // Determine status message based on priority
            String statusMsg;

            if (EStop::getInstance()->isTriggered())
            {
                statusMsg = "⚠️ WIRELESS DISABLED (eStop) - No messages being sent";
                statusControl->color = ControlColor::Alizarin; // Red
            }
            else if (!WIRELESS_ENABLED)
            {
                statusMsg = "⚠️ WIRELESS DISABLED - No messages being sent";
                statusControl->color = ControlColor::Alizarin; // Red
            }
            else if (!GAME_ENABLED)
            {
                statusMsg = "⚠️ GAME DISABLED - Game inputs ignored";
                statusControl->color = ControlColor::Alizarin; // Red
            }
            else if (SIMONA_CHEAT_MODE)
            {
                statusMsg = "⚠️ CHEAT MODE ENABLED - Game sequence predictable ⚠️";
                statusControl->color = ControlColor::Sunflower; // Yellow
            }
            else
            {
                // Check packet loss
                float lossPercentage = getPacketLossPercentage();
                if (lossPercentage > 2.0)
                {
                    statusMsg = String("⚠️ HIGH PACKET LOSS: ") + String(lossPercentage, 1) + "% in the last 5 minutes";
                    statusControl->color = ControlColor::Peterriver; // Blue
                }
                else
                {
                    statusMsg = "🐹 System Normal 🐹";
                    statusControl->color = ControlColor::Emerald; // Green
                }
            }

            ESPUI.updateControlValue(status, statusMsg);
            ESPUI.updateControl(statusControl);
        }

        lastStatusUpdate = millis();
    }

    // Update packet loss stats every second
    if (millis() - lastPacketLossUpdate >= 1000)
    {
        float lossPercentage = getPacketLossPercentage();
        int totalSent = getTotalMessagesSent();
        int totalLost = getTotalMessagesLost();
        int windowMessages = getMessagesInTimeWindow(); // Get messages in 5-min window

        char buffer[100];
        snprintf(buffer, sizeof(buffer),
                 "Last 5min: %.2f%% loss (%d msgs) | Total: %.2f%% loss (%d/%d msgs)",
                 lossPercentage,
                 windowMessages, // Now shows actual count in the window
                 totalSent > 0 ? (totalLost * 100.0) / totalSent : 0.0,
                 totalLost,
                 totalSent);

        ESPUI.updateControlValue(packetLossLabel, buffer);

        // Set color based on loss percentage
        Control *lossControl = ESPUI.getControl(packetLossLabel);
        if (lossControl)
        {
            if (lossPercentage < 1.0)
            {
                lossControl->color = ControlColor::Emerald; // Good - less than 1%
            }
            else if (lossPercentage < 5.0)
            {
                lossControl->color = ControlColor::Sunflower; // Warning - 1-5%
            }
            else
            {
                lossControl->color = ControlColor::Alizarin; // Bad - more than 5%
            }
            ESPUI.updateControl(lossControl);
        }

        lastPacketLossUpdate = millis();
    }

    // Rest of timing logic for main updates
    unsigned long currentMillis = millis();
    unsigned long seconds = (currentMillis / 1000) % 60;
    unsigned long minutes = (currentMillis / (1000 * 60)) % 60;
    unsigned long hours = (currentMillis / (1000 * 60 * 60)) % 24;
    unsigned long days = (currentMillis / (1000 * 60 * 60 * 24));

    String formattedTime = String(days) + "d " + String(hours) + "h " + String(minutes) + "m " + String(seconds) + "s";

    // Update controls every 500ms (twice per second) instead of every 1000ms
    if (millis() - oldTime > 500) // Changed from 1000 to 500
    {
        // Toggle switch state
        switchState = !switchState;

        // Update switch and millis controls
        ESPUI.updateControlValue(controlMillis, formattedTime);

        // Update Simona progress using singleton
        Simona *simona = Simona::getInstance();
        if (simona)
        { // Changed this to get singleton instance
            String progressStr;
            if (simona->getCurrentStage() == SIMONA_STAGE_WAITING)
            {
                progressStr = "Waiting...";
            }
            else
            {
                progressStr = "Round: " + String(simona->getCurrentRound()) + "/" +
                              String(simona->getMaxRounds()) + " - Level: " +
                              String(simona->getCurrentLevel()) + "/" +
                              String(simona->getLevelsInRound());
            }
            ESPUI.updateControlValue(simonaProgressLabel, progressStr);

            // Update Simona next color
            const char *expectedColor = simona->getExpectedColor();
            String colorStr = expectedColor ? String("Expecting: ") + expectedColor : "Waiting for sequence...";
            ESPUI.updateControlValue(simonaNextColorLabel, colorStr);
            Control *colorControl = ESPUI.getControl(simonaNextColorLabel);
            if (colorControl)
            {
                colorControl->color = getColorForName(expectedColor);
                ESPUI.updateControl(colorControl);
            }

            // Update Simona time remaining
            uint32_t timeRemaining = simona->getInputTimeRemaining();
            String timeStr;
            if (timeRemaining > 0)
            {
                timeStr = String(timeRemaining) + " seconds remaining";
            }
            else if (simona->getCurrentStage() == SIMONA_STAGE_INPUT_COLLECTION)
            {
                timeStr = "Time's up!";
            }
            else
            {
                timeStr = "-";
            }
            ESPUI.updateControlValue(simonaTimeRemainingLabel, timeStr);
        }

        // Update oldTime
        oldTime = millis();
    }
}

// Task definition moved to Tasks.cpp