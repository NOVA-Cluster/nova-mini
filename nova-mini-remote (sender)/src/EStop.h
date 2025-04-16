#ifndef ESTOP_H
#define ESTOP_H

#include <Arduino.h>
#include "configuration.h"

class EStop {
public:
    static EStop* getInstance();
    void begin();
    bool isTriggered() const;

private:
    EStop() = default;
    static EStop* instance;
    bool triggered = false;
    uint32_t lastDebounceTime = 0;  // Last time the input pin was toggled
    
    // LED pulsing variables
    uint8_t pulseValue = 0;
    int8_t pulseDelta = 1;  // Amount to change brightness by
    uint32_t lastPulseTime = 0;
    static constexpr uint8_t MIN_BRIGHTNESS = 0;
    static constexpr uint8_t MAX_BRIGHTNESS = 255;
    static constexpr uint16_t PULSE_INTERVAL_MS = 2;  // Time between brightness updates
    
    // Previous LED states - initialized to off state (255 since active LOW)
    uint8_t prevRedState = 255;
    uint8_t prevGreenState = 255;
    uint8_t prevBlueState = 255;
    uint8_t prevYellowState = 255;
    uint8_t prevResetState = 255;
    
    void checkEStopStatus();
    void updateLEDPulse();  // New method to handle LED pulsing
    void setOutputLEDs(uint8_t brightness);  // New method to control LED brightness
    void storeLEDStates();  // Store current LED states
    void restoreLEDStates();  // Restore previous LED states

    friend void eStopTask(void* pvParameters);
};

#endif // ESTOP_H
