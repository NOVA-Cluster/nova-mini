#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>

// Task function declarations
void gameTask(void *pvParameters);
void buttonTask(void *pvParameters);
void eStopTask(void *pvParameters);
void espNowTask(void *pvParameters);
void TaskWiFiConnection(void *pvParameters);
void runEspuiTask(void *pvParameters);

// Function to create and start all tasks
void createTasks(void);

#endif // TASKS_H