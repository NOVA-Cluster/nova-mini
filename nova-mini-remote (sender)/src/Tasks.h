#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>

// Task function declarations
void gameTask(void *pvParameters);
void buttonTask(void *pvParameters);
void eStopTask(void *pvParameters);
void espNowTask(void *pvParameters);

#endif // TASKS_H