#include "Tasks.h"
#include "utilities/utilities.h"  // Updated path
#include "configuration.h"
#include "Simona.h"
#include "midi/MIDIControl.hpp"  // Updated path to MIDI module
#include <MIDI.h>
#include "EspNow.h"
#include <WiFi.h>
#include <WiFiType.h>
#include "Web.h"
#include "EStop.h"  // Added for E-Stop functionality

// Task definitions
void gameTask(void *pvParameters) {
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);
    
    safeSerialPrintf("Game task is running on core %d\n", xPortGetCoreID());
    
    while (true) {
        // Run the Simona game logic
        Simona::getInstance()->runGameTask();
        
        // Allow other tasks to execute
        yield();
        delay(1);
        
        // Report task status periodically
        static uint32_t lastExecutionTime = 0;
        if (millis() - lastExecutionTime >= REPORT_TASK_INTERVAL) {
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = millis();
        }
        
        // Add a delay to prevent watchdog triggers
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void buttonTask(void *pvParameters) {
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);
    
    safeSerialPrintf("Button task is running on core %d\n", xPortGetCoreID());
    
    while (true) {
        // Process button inputs for the Simona game
        Simona::getInstance()->runButtonTask();
        
        // Allow other tasks to execute
        yield();
        delay(1);
        
        // Report task status periodically
        static uint32_t lastExecutionTime = 0;
        if (millis() - lastExecutionTime >= REPORT_TASK_INTERVAL) {
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = millis();
        }
        
        // Add a small delay to prevent watchdog triggers
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void eStopTask(void *pvParameters) {
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);
    
    safeSerialPrintf("E-Stop task is running on core %d\n", xPortGetCoreID());
    
    // Initialize the E-Stop instance
    EStop::getInstance()->begin();
    
    while (true) {
        // Check E-Stop status
        EStop::getInstance()->checkEStopStatus();
        
        // Report task status periodically
        static uint32_t lastExecutionTime = 0;
        if (millis() - lastExecutionTime >= REPORT_TASK_INTERVAL) {
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = millis();
        }
        
        // Wait for next check interval
        vTaskDelay(pdMS_TO_TICKS(ESTOP_TASK_INTERVAL));
    }
}