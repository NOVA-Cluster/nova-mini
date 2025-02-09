#include "EspNow.h"
#include <WiFi.h>
#include <esp_now.h>
#include "main.h" // For triggerRelay(), etc.
#include "PooferMessage.h"

PooferMessage currentMessage;  // Global, if needed for other logic

void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    if(len == sizeof(PooferMessage)) {
        PooferMessage receivedMsg;
        memcpy(&receivedMsg, incomingData, sizeof(PooferMessage));
        
        // Ensure relay is triggered only once per unique messageId.
        static uint32_t lastMessageId = 0;
        if(receivedMsg.messageId == lastMessageId) {
            Serial.println("Duplicate message received, ignoring.");
            return;
        }
        lastMessageId = receivedMsg.messageId;
        
        Serial.print("Received messageId: ");
        Serial.println(receivedMsg.messageId);

        // Process LED outputs for each poofer
        for(int i = 0; i < 4; i++){
            if(receivedMsg.poofers[i].duration > 0){
                triggerRelay(i, receivedMsg.poofers[i].duration);
                // Optionally use receivedMsg.poofers[i].red, receivedMsg.poofers[i].green, receivedMsg.poofers[i].blue to drive an LED.
            }
        }

        // Send ack back to sender using its MAC
        esp_err_t result = esp_now_send(mac, (uint8_t *)&receivedMsg, sizeof(PooferMessage));
        if(result == ESP_OK){
            Serial.println("Ack sent");
        } else {
            Serial.println("Error sending ack");
        }
    }
}

void initEspNowReceiver() {
    Serial.begin(115200);
    WiFi.mode(WIFI_AP);
    if(esp_now_init() != ESP_OK){
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(onDataRecv);
    Serial.println("ESP-NOW Receiver initialized");
}