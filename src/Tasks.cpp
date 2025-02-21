#include "Tasks.h"
#include "Utilities.h"
#include "configuration.h"
#include "main.h"
#include <FastLED.h>
#include "SimonaDisplaySequences.h" // Added include for LED animation functions
// ...other necessary includes...

// TaskWeb definition
void TaskWeb(void *pvParameters)
{
    // ...existing setup code from TaskWeb...
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);
    safeSerialPrintf("TaskWeb is running\n");
    while (1)
    {
        // ...existing code...
        yield();
        delay(1);
        static uint32_t lastExecutionTime = 0;
        if (millis() - lastExecutionTime >= REPORT_TASK_INTERVAL)
        {
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = millis();
        }
    }
}

// TaskOutputs definition
void TaskOutputs(void *pvParameters)
{
    // ...existing code from TaskOutputs...
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);
    safeSerialPrintf("TaskOutputs is running\n");
    while (1)
    {
        uint32_t currentTime = millis();
        // ...existing relay control code...
        for (int i = 0; i < NUM_RELAYS; i++)
        {
            if (relayTasks[i].active)
            {
                if (relayTasks[i].longActive)
                {
                    if (currentTime >= relayTasks[i].offTime)
                    {
                        sr.set(i, LOW);
                        relayTasks[i].active = false;
                    }
                }
                else
                {
                    if (currentTime >= relayTasks[i].offTime ||
                        (currentTime - relayTasks[i].startTime) >= MAX_RELAY_DURATION)
                    {
                        sr.set(i, LOW);
                        relayTasks[i].active = false;
                    }
                }
            }
        }
        // ...other existing code...
        yield();
        delay(1);
        static uint32_t lastExecutionTime = 0;
        if (millis() - lastExecutionTime >= REPORT_TASK_INTERVAL)
        {
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = millis();
        }
    }
}

// TaskPulseRelay definition
void TaskPulseRelay(void *pvParameters)
{
    // ...existing code from TaskPulseRelay...
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);
    safeSerialPrintf("TaskPulseRelay is running\n");
    while (1)
    {
        uint32_t currentTime = millis();
        static uint32_t lastPulse = 0;
        if (currentTime - lastPulse >= 1000)
        {
            triggerRelay(7, 100);
            lastPulse = currentTime;
        }
        yield();
        delay(1);
        static uint32_t lastExecutionTime = 0;
        if (currentTime - lastExecutionTime >= REPORT_TASK_INTERVAL)
        {
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = currentTime;
        }
    }
}

// TaskFastLED definition
void TaskFastLED(void *pvParameters)
{
    UBaseType_t uxHighWaterMark;
    TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
    const char *pcTaskName = pcTaskGetName(xTaskHandle);
    safeSerialPrintf("TaskFastLED is running\n");
    
    uint32_t lastExecutionTime = 0; // new watermark timer
    for (;;) {
        switch (currentLEDAnimationState) {
            case LED_WAITING:
                displaySimonaStageWaitingAnimation();
                break;
            case LED_SEQUENCE_GENERATION:
                displaySimonaStageSequenceGenerationAnimation();
                break;
            case LED_TRANSITION:
                displaySimonaStageTransitionAnimation();
                break;
            case LED_INPUT_COLLECTION:
                displaySimonaStageInputCollectionAnimation();
                break;
            case LED_VERIFICATION:
                displaySimonaStageVerificationAnimation();
                break;
            case LED_GAME_LOST:
                displaySimonaStageGameLostAnimation();
                break;
            case LED_GAME_WIN:
                displaySimonaStageGameWinAnimation();
                break;
            case LED_RESET:
                displaySimonaStageResetAnimation();
                break;
            default:
                // ...existing default code...
                break;
        }
        if (millis() - lastExecutionTime >= REPORT_TASK_INTERVAL) {
            uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            safeSerialPrintf("%s stack free - %d running on core %d\n", pcTaskName, uxHighWaterMark, xPortGetCoreID());
            lastExecutionTime = millis();
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
