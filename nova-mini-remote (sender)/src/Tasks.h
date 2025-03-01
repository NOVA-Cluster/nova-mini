#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>

// Task function declarations
void gameTask(void *pvParameters);
void buttonTask(void *pvParameters);

#endif // TASKS_H