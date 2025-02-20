#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>

void TaskWeb(void *pvParameters);
void TaskOutputs(void *pvParameters);
void TaskPulseRelay(void *pvParameters);
void TaskFastLED(void *pvParameters);

#endif // TASKS_H
