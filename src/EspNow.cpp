#include "EspNow.h"
#include <WiFi.h>
#include <esp_now.h>
#include "main.h"          // For triggerRelay(), etc.
#include "SimonaMessage.h" // Changed from PooferMessage.h
#include "SimonaTypes.h"   // Added for SimonaTypes

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
    case SIMONA_STAGE_RESET:              // Added for reset stage.
        return "SIMONA_STAGE_RESET";
    default:
        return "UNKNOWN";
    }
}

SimonaMessage currentMessage; // Global variable updated to SimonaMessage

void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
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

        safeSerialPrintf("Received message_id: %d\n", receivedMsg.message_id);

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

        // Removed LED processing loop since SimonaMessage has no 'poofers'

        // Send ack back to sender using its MAC
        esp_err_t result = esp_now_send(mac, (uint8_t *)&receivedMsg, sizeof(SimonaMessage));
        if (result == ESP_OK)
        {
            safeSerialPrintf("Ack sent\n");
        }
        else
        {
            // Serial.println("Error sending ack");
        }
    }
}

void initEspNowReceiver()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_AP);
    if (esp_now_init() != ESP_OK)
    {
        safeSerialPrintf("Error initializing ESP-NOW\n");
        return;
    }
    esp_now_register_recv_cb(onDataRecv);
    safeSerialPrintf("ESP-NOW Receiver initialized\n");
}