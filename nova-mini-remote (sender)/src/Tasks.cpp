#include "Tasks.h"
#include <WiFi.h>
#include "configuration.h"
#include "utilities/utilities.h"     // For safeSerialPrintf
#include "Simona.h"                 // For Simona class
#include "EStop.h"                  // For EStop class
#include "EspNow.h"                 // For espNowLoop function
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

void espNowTask(void *pvParameters) {
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);
    
    safeSerialPrintf("ESP-NOW task is running on core %d\n", xPortGetCoreID());
    
    while (true) {
        // Run ESP-NOW loop operations
        espNowLoop();
        
        // Report task status periodically
        static uint32_t lastExecutionTime = 0;
        if (millis() - lastExecutionTime >= REPORT_TASK_INTERVAL) {
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = millis();
        }
        
        // Use a 1ms delay which is typical for network communication tasks
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void runEspuiTask(void *pvParameters) {
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);
    
    safeSerialPrintf("ESPUI task is running on core %d\n", xPortGetCoreID());
    
    while (true) {
        webLoop();
        
        // Report task status periodically
        static uint32_t lastExecutionTime = 0;
        if (millis() - lastExecutionTime >= REPORT_TASK_INTERVAL) {
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = millis();
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void createTasks(void) {
    // Create game and input handling tasks
    xTaskCreate(gameTask, "Game Task", 4096, NULL, 1, NULL);
    xTaskCreate(buttonTask, "Button Task", 4096, NULL, 1, NULL);
    xTaskCreate(eStopTask, "E-Stop Task", 2048, NULL, 2, NULL);
    xTaskCreate(espNowTask, "ESP-NOW Task", 4096, NULL, 1, NULL);

    // Create WiFi monitoring task
    xTaskCreate(TaskWiFiConnection, "WiFi Connection", 4096, NULL, 1, NULL);

    // Create ESPUI web interface task
    xTaskCreate(runEspuiTask, "ESPUI", 8192, NULL, 1, NULL);
}

void TaskWiFiConnection(void *pvParameters)
{
    uint32_t lastExecutionTime = 0;

    Serial.println("TaskWiFiConnection is running");

    while (1)
    {
        // Only monitor AP status since we're not connecting to other networks
        uint8_t stationNum = WiFi.softAPgetStationNum();
        
        if (millis() - lastExecutionTime >= REPORT_TASK_INTERVAL)
        {
            Serial.printf("AP Status - Connected stations: %d\n", stationNum);
            lastExecutionTime = millis();
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Check status every 2 seconds
    }
}