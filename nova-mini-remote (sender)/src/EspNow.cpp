#include <WiFi.h>
#include <esp_now.h>
#include "SimonaMessage.h"
#include "configuration.h"
#include <Preferences.h>  // Add this include
#include "EspNow.h"       // Make sure we include our own header file
#include "utilities/PreferencesManager.h" // Add this include
#include "EStop.h"        // Include EStop header for E-Stop functionality

static portMUX_TYPE espNowMux = portMUX_INITIALIZER_UNLOCKED;

bool ackReceived = false;
uint32_t currentMessageId = 0;
// Replace the broadcast address with a target address
uint8_t targetAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};  // Will be updated with saved address
bool espNowInitialized = false;

// Add packet loss tracking
#define PACKET_HISTORY_SIZE 1000  // Store up to 1000 packet statuses
#define PACKET_WINDOW_MS (5 * 60 * 1000)  // 5 minutes in milliseconds

struct PacketStatus {
    uint32_t timestamp;
    bool ackReceived;
};

// Circular buffer for packet history
static PacketStatus packetHistory[PACKET_HISTORY_SIZE];
static int packetHistoryIndex = 0;
static int packetHistoryCount = 0;
static int totalMessagesSent = 0;
static int totalMessagesLost = 0;

// Add a packet to the history
void addPacketStatus(bool ackReceived) {
    portENTER_CRITICAL(&espNowMux);
    
    packetHistory[packetHistoryIndex].timestamp = millis();
    packetHistory[packetHistoryIndex].ackReceived = ackReceived;
    
    totalMessagesSent++;
    if (!ackReceived) {
        totalMessagesLost++;
    }
    
    // Move to next position in circular buffer
    packetHistoryIndex = (packetHistoryIndex + 1) % PACKET_HISTORY_SIZE;
    
    // Update count until buffer is full
    if (packetHistoryCount < PACKET_HISTORY_SIZE) {
        packetHistoryCount++;
    }
    
    portEXIT_CRITICAL(&espNowMux);
}

float getPacketLossPercentage() {
    uint32_t currentTime = millis();
    int windowPackets = 0;
    int windowLost = 0;

    portENTER_CRITICAL(&espNowMux);
    
    // Count packets in the 5-minute window
    for (int i = 0; i < packetHistoryCount; i++) {
        int idx = (packetHistoryIndex - 1 - i + PACKET_HISTORY_SIZE) % PACKET_HISTORY_SIZE;
        
        // Check if packet is within the last 5 minutes, accounting for millis() overflow
        uint32_t packetAge;
        if (currentTime >= packetHistory[idx].timestamp) {
            packetAge = currentTime - packetHistory[idx].timestamp;
        } else {
            // millis() has overflowed
            packetAge = (0xFFFFFFFF - packetHistory[idx].timestamp) + currentTime + 1;
        }
        
        // Only count packets within the 5-minute window
        if (packetAge <= PACKET_WINDOW_MS) {
            windowPackets++;
            if (!packetHistory[idx].ackReceived) {
                windowLost++;
            }
        }
    }
    
    portEXIT_CRITICAL(&espNowMux);
    
    // Calculate percentage
    if (windowPackets > 0) {
        return (windowLost * 100.0) / windowPackets;
    }
    return 0.0;  // No packets in window
}

// Add this new function to count messages in the time window
int getMessagesInTimeWindow() {
    uint32_t currentTime = millis();
    int windowPackets = 0;

    portENTER_CRITICAL(&espNowMux);
    
    // Count packets in the 5-minute window
    for (int i = 0; i < packetHistoryCount; i++) {
        int idx = (packetHistoryIndex - 1 - i + PACKET_HISTORY_SIZE) % PACKET_HISTORY_SIZE;
        
        // Check if packet is within the last 5 minutes, accounting for millis() overflow
        uint32_t packetAge;
        if (currentTime >= packetHistory[idx].timestamp) {
            packetAge = currentTime - packetHistory[idx].timestamp;
        } else {
            // millis() has overflowed
            packetAge = (0xFFFFFFFF - packetHistory[idx].timestamp) + currentTime + 1;
        }
        
        // Only count packets within the 5-minute window
        if (packetAge <= PACKET_WINDOW_MS) {
            windowPackets++;
        }
    }
    
    portEXIT_CRITICAL(&espNowMux);
    
    return windowPackets;
}

int getTotalMessagesSent() {
    return totalMessagesSent;
}

int getTotalMessagesLost() {
    return totalMessagesLost;
}

// Add this helper function to convert MAC string to bytes
bool macStringToBytes(const String& macStr, uint8_t* bytes) {
    if (macStr.length() != 17) return false;  // XX:XX:XX:XX:XX:XX
    
    for (int i = 0; i < 6; i++) {
        String byteStr = macStr.substring(i*3, i*3 + 2);
        bytes[i] = (uint8_t)strtol(byteStr.c_str(), NULL, 16);
    }
    return true;
}

// Callback function invoked when ESP-NOW data is received.
// This function sets the ACK flag to true.
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
    Serial.println("Data received, setting ACK flag.");
    portENTER_CRITICAL(&espNowMux);
    ackReceived = true;
    portEXIT_CRITICAL(&espNowMux);
}

// Initializes and sets up ESP-NOW functionality.
// Registers the receive callback and adds a broadcast peer.
void espNowSetup()
{
    // Load saved MAC address
    String savedMac = PreferencesManager::getString(PREF_KEY_RECEIVER_MAC, "");

    // Update target address if we have a saved MAC
    if (savedMac.length() > 0) {
        if (macStringToBytes(savedMac, targetAddress)) {
            Serial.printf("Using saved receiver MAC: %s\n", savedMac.c_str());
        }
    }

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    espNowInitialized = true;
    esp_now_register_recv_cb(onDataRecv);

    // Update peer info to use target address instead of broadcast
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, targetAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    // Remove existing peer if any
    esp_now_del_peer(targetAddress);
    
    // Add the peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }
}

// Returns a string representing the name of a SimonaStage enum value.
// Helps in debugging and logging message stages.
const char* getSimonaStageName(SimonaStage stage) {
    switch (stage) {
        case SIMONA_STAGE_WAITING:               return "SIMONA_STAGE_WAITING";
        case SIMONA_STAGE_SEQUENCE_GENERATION:   return "SIMONA_STAGE_SEQUENCE_GENERATION";
        case SIMONA_STAGE_TRANSITION:            return "SIMONA_STAGE_TRANSITION";
        case SIMONA_STAGE_INPUT_COLLECTION:      return "SIMONA_STAGE_INPUT_COLLECTION";
        case SIMONA_STAGE_VERIFICATION:          return "SIMONA_STAGE_VERIFICATION";
        case SIMONA_STAGE_GAME_LOST:             return "SIMONA_STAGE_GAME_LOST";
        case SIMONA_STAGE_GAME_WIN:              return "SIMONA_STAGE_GAME_WIN";
        case SIMONA_STAGE_RESET:                 return "SIMONA_STAGE_RESET";
        case SIMONA_STAGE_ROUND_TRANSITION:      return "SIMONA_STAGE_ROUND_TRANSITION"; // Added missing stage
        default:                                 return "UNKNOWN_STAGE";
    }
}

// Prints the details of a SimonaMessage for debugging purposes.
// Displays each field of the message using Serial.printf.
void printSimonaMessage(const SimonaMessage &msg)
{
    Serial.printf("Sending SimonaMessage:\n  message_id: %d\n  stage: %d (%s)\n  level: %d\n  gamePlay: %d\n  lost: %d\n  litButton: %d\n  lastPressedButton: %d\n",
                  msg.message_id, msg.stage, getSimonaStageName(msg.stage),
                  msg.level, msg.gamePlay, msg.lost, msg.litButton, msg.lastPressedButton);
}

// Sends a SimonaMessage using ESP-NOW with retry logic if ACK is not received.
// This function sends the message, waits for an ACK for a specified timeout,
// and resends the message up to MAX_RETRIES times if no ACK is received.
void sendSimonaMessage(const SimonaMessage &simMsg)
{
    // Check if wireless is disabled
    if (!WIRELESS_ENABLED) {
        Serial.println("Wireless disabled - not sending message");
        addPacketStatus(false);  // Count as lost packet
        return;
    }

    // Check if E-Stop is triggered
    if (EStop::getInstance()->isTriggered()) {
        Serial.println("E-Stop triggered - not sending message");
        addPacketStatus(false);  // Count as lost packet
        return;
    }
    
    if (!espNowInitialized)
    {
        Serial.println("ESP-NOW not initialized, calling espNowSetup()...");
        espNowSetup();
        if (!espNowInitialized)
        {
            Serial.println("Failed to initialize ESP-NOW");
            addPacketStatus(false);  // Count as lost packet
            return;
        }
    }
    
    ackReceived = false;
    currentMessageId++;
    SimonaMessage msgToSend = simMsg;
    msgToSend.message_id = currentMessageId;
    
    // Log message details before sending.
    printSimonaMessage(msgToSend);
    
    // Update to use targetAddress instead of broadcastAddress
    esp_err_t result = esp_now_send(targetAddress, (uint8_t *)&msgToSend, sizeof(SimonaMessage));
    if (result == ESP_OK)
        Serial.println("Message sent, awaiting ACK...");
    else {
        Serial.println("Error sending message.");
        addPacketStatus(false);  // Count as lost packet
        return;
    }
    
    uint8_t retries = 0;
    // Retry loop: wait ACK_TIMEOUT_MS for an ACK, then resend if not received.
    while (!ackReceived && retries < MAX_RETRIES)
    {
        uint32_t startTime = millis();
        while (millis() - startTime < ACK_TIMEOUT_MS)
        {
            if (ackReceived)
                break;
            vTaskDelay(1);
        }
        if (ackReceived)
            break;

        retries++;
        result = esp_now_send(targetAddress, (uint8_t *)&msgToSend, sizeof(SimonaMessage));
        if (result != ESP_OK)
        {
            Serial.println("Error resending message.");
            break;
        }
    }
    
    // Record packet status after all retries
    addPacketStatus(ackReceived);
    
    if (!ackReceived) {
        if (retries > 0) {
            Serial.printf("No ACK received after %d retries\n", retries);
        } else {
            Serial.println("No ACK received");
        }
    } else {
        Serial.println("ACK received for sent message.");
    }
        
    vTaskDelay(1);
}

// Main loop for ESP-NOW tasks.
// This function can be expanded for regular ESP-NOW operations.
void espNowLoop()
{
    vTaskDelay(1);
}
