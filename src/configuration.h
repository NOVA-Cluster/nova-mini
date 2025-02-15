/*
 * @Description: None
 * @version: None
 * @Author: None
 * @Date: 2023-06-05 13:01:59
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2024-07-19 09:15:01
 */
#pragma once

// HT74HC595
#define HT74HC595_CLOCK 5
#define HT74HC595_LATCH 6
#define HT74HC595_DATA 7
#define HT74HC595_OUT_EN 4

// New configuration values moved from main.cpp:
#define NUMBER_OF_SHIFT_REGISTERS 1
#define MAX_RELAY_DURATION 250

// Delay to wait for serial monitor to open (ms)
#define SERIAL_MONITOR_DELAY_MS 3000

// Reset pin configuration
#define RESET_PIN 39
#define RESET_LOW_DELAY_MS 50

#define REPORT_TASK_INTERVAL (120 * 1000) // How often to report task status in milliseconds
