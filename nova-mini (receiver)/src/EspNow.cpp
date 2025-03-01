#include "EspNow.h"
#include <WiFi.h>
#include <esp_now.h>
#include "main.h"          // For triggerRelay(), etc.
#include "SimonaMessage.h" // Changed from PooferMessage.h
#include "SimonaTypes.h"   // Added for SimonaTypes
#include "SimonaDisplay.h" // Added to declare display functions
#include <ESPUI.h>         // Added for ESPUI
#include "PreferencesManager.h" // Added to use PreferencesManager instead of direct Preferences

// Helper function to convert SimonaStage enum to a string.
const char *stageToString(SimonaStage stage)
{
    switch (stage)
    {
    case SIMONA_STAGE_WAITING:
        return "SIMONA_STAGE_WAITING";
    case SIMONA_STAGE_SEQUENCE_GENERATION:
        return "SIMONA_STAGE_SEQUENCE_GENERATION";
    case SIMONA_STAGE_TRANSITION:
        return "SIMONA_STAGE_TRANSITION";
    case SIMONA_STAGE_INPUT_COLLECTION:
        return "SIMONA_STAGE_INPUT_COLLECTION";
    case SIMONA_STAGE_VERIFICATION:
        return "SIMONA_STAGE_VERIFICATION";
    case SIMONA_STAGE_GAME_LOST:
        return "SIMONA_STAGE_GAME_LOST";
    case SIMONA_STAGE_GAME_WIN:
        return "SIMONA_STAGE_GAME_WIN";
    case SIMONA_STAGE_RESET: // Added for reset stage.
        return "SIMONA_STAGE_RESET";
    case SIMONA_STAGE_ROUND_TRANSITION: // Added for round transition
        return "SIMONA_STAGE_ROUND_TRANSITION";
    default:
        return "UNKNOWN";
    }
}

SimonaMessage currentMessage; // Global variable updated to SimonaMessage

// Add global variable to track connected remotes
extern uint16_t connectedRemotesLabel;
String connectedRemotes = "No remotes connected";

void updatePeerList(const uint8_t *mac)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // Save this MAC using PreferencesManager with key KEY_REMOTE_MAC
    PreferencesManager::setString(PreferencesManager::KEY_REMOTE_MAC, macStr);

    if (connectedRemotes.indexOf(macStr) == -1)
    {
        if (connectedRemotes == "No remotes connected")
        {
            connectedRemotes = String(macStr);
        }
        else
        {
            connectedRemotes += "\n" + String(macStr);
        }
        ESPUI.updateControlValue(connectedRemotesLabel, connectedRemotes);
    }
}

void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    // Update and save the remote's MAC address
    updatePeerList(mac);

    // Pulse relay 6 on every message receipt
    triggerRelay(6, 50); // 50ms pulse duration

    if (len == sizeof(SimonaMessage))
    {
        SimonaMessage receivedMsg;
        memcpy(&receivedMsg, incomingData, sizeof(SimonaMessage));

        // Ensure relay is triggered only once per unique message_id.
        static uint32_t lastMessageId = 0;
        if (receivedMsg.message_id == lastMessageId)
        {
            safeSerialPrintf("Duplicate message received, ignoring.\n");
            return;
        }
        lastMessageId = receivedMsg.message_id;

        safeSerialPrintf("*** Received message_id: %d ***\n", receivedMsg.message_id);

                // Print full message content in human readable form.
                safeSerialPrintf("Message contents:\n");
                safeSerialPrintf("  message_id: %d\n", receivedMsg.message_id);
                // Updated to print the enum name
                safeSerialPrintf("  stage: %s\n", stageToString(receivedMsg.stage));
                safeSerialPrintf("  level: %d\n", receivedMsg.level);
                safeSerialPrintf("  gamePlay: %d\n", receivedMsg.gamePlay);
                safeSerialPrintf("  lost: %d\n", receivedMsg.lost);
                safeSerialPrintf("  litButton: %d\n", receivedMsg.litButton);
                safeSerialPrintf("  lastPressedButton: %d\n", receivedMsg.lastPressedButton);
        /*
        */

        // Call the display function for the corresponding stage.
        switch (receivedMsg.stage)
        {
        case SIMONA_STAGE_WAITING:
            displaySimonaStageWaiting(receivedMsg);
            break;
        case SIMONA_STAGE_SEQUENCE_GENERATION:
            displaySimonaStageSequenceGeneration(receivedMsg);
            break;
        case SIMONA_STAGE_TRANSITION:
            displaySimonaStageTransition(receivedMsg);
            break;
        case SIMONA_STAGE_INPUT_COLLECTION:
            displaySimonaStageInputCollection(receivedMsg);
            break;
        case SIMONA_STAGE_VERIFICATION:
            displaySimonaStageVerification(receivedMsg);
            break;
        case SIMONA_STAGE_GAME_LOST:
            displaySimonaStageGameLost(receivedMsg);
            break;
        case SIMONA_STAGE_GAME_WIN:
            displaySimonaStageGameWin(receivedMsg);
            break;
        case SIMONA_STAGE_RESET:
            displaySimonaStageReset(receivedMsg);
            break;
        case SIMONA_STAGE_ROUND_TRANSITION:
            displaySimonaStageRoundTransition(receivedMsg);
            break;
        default:
            safeSerialPrintf("Unknown stage.\n");
            break;
        }

        // Send ack back to sender using its MAC
        esp_err_t result = esp_now_send(mac, (uint8_t *)&receivedMsg, sizeof(SimonaMessage));
        if (result == ESP_OK)
        {
            safeSerialPrintf("Ack sent\n");
        }
        else
        {
            char macStr[18];
            snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            safeSerialPrintf("Error sending ack to %s: %s\n", macStr, esp_err_to_name(result));
        }
    }
    else
    {
        safeSerialPrintf("*** Received message *** : Wrong length. Received: %d, Expected: %d\n", len, sizeof(SimonaMessage));

    }
}

void initEspNowReceiver()
{
    if (esp_now_init() != ESP_OK)
    {
        safeSerialPrintf("Error initializing ESP-NOW\n");
        return;
    }

    // Load MAC address from PreferencesManager using KEY_REMOTE_MAC
    String remoteMacAddress = PreferencesManager::getString(PreferencesManager::KEY_REMOTE_MAC, "");

    if (!remoteMacAddress.isEmpty())
    {
        // Convert MAC string to bytes
        uint8_t remoteMac[6];
        sscanf(remoteMacAddress.c_str(), "%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX",
               &remoteMac[0], &remoteMac[1], &remoteMac[2],
               &remoteMac[3], &remoteMac[4], &remoteMac[5]);

        // Add peer using saved MAC
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, remoteMac, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;

        esp_now_del_peer(remoteMac); // Remove if exists
        esp_err_t addPeerResult = esp_now_add_peer(&peerInfo);
        if (addPeerResult == ESP_OK)
        {
            safeSerialPrintf("ESP-NOW peer configured: %s\n", remoteMacAddress.c_str());
            // Update the UI
            connectedRemotes = remoteMacAddress;
            ESPUI.updateControlValue(connectedRemotesLabel, connectedRemotes);
        }
        else
        {
            safeSerialPrintf("Failed to add ESP-NOW peer: %s\n", esp_err_to_name(addPeerResult));
        }
    }
    else
    {
        safeSerialPrintf("No remote MAC address configured\n");
    }

    esp_now_register_recv_cb(onDataRecv);
    safeSerialPrintf("ESP-NOW Receiver initialized\n");
}