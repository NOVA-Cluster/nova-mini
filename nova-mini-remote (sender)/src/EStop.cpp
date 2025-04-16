#include "EStop.h"
#include "configuration.h"
#include "utilities/utilities.h"

EStop* EStop::instance = nullptr;

EStop* EStop::getInstance() {
    if (instance == nullptr) {
        instance = new EStop();
    }
    return instance;
}

void EStop::begin() {
    pinMode(ESTOP_PIN, INPUT_PULLUP);
    triggered = false;
    
    // Initialize LED pins to on state (0 since LEDs are active LOW)
    setOutputLEDs(0);  // 0 = full brightness = LEDs on
}

bool EStop::isTriggered() const {
    return triggered;
}

void EStop::setOutputLEDs(uint8_t brightness) {
    // Invert brightness since LEDs are active LOW
    uint8_t invertedBrightness = 255 - brightness;
    ledcWrite(LEDC_CHANNEL_RED, invertedBrightness);
    ledcWrite(LEDC_CHANNEL_GREEN, invertedBrightness);
    ledcWrite(LEDC_CHANNEL_BLUE, invertedBrightness);
    ledcWrite(LEDC_CHANNEL_YELLOW, invertedBrightness);
    ledcWrite(LEDC_CHANNEL_RESET, invertedBrightness);
}

void EStop::updateLEDPulse() {
    uint32_t currentTime = millis();
    if (currentTime - lastPulseTime >= PULSE_INTERVAL_MS) {
        lastPulseTime = currentTime;
        
        // Update pulse value
        pulseValue += pulseDelta;
        
        // Change direction at limits
        if (pulseValue >= MAX_BRIGHTNESS) {
            pulseValue = MAX_BRIGHTNESS;
            pulseDelta = -1;
        } else if (pulseValue <= MIN_BRIGHTNESS) {
            pulseValue = MIN_BRIGHTNESS;
            pulseDelta = 1;
        }
        
        setOutputLEDs(pulseValue);
    }
}

void EStop::checkEStopStatus() {
    bool currentState = (digitalRead(ESTOP_PIN) == LOW);
    uint32_t currentTime = millis();

    // Only update the state if enough time has passed since the last state change
    if ((currentTime - lastDebounceTime) > ESTOP_DEBOUNCE_MS) {
        if (currentState != triggered) {
            if (currentState) {  // E-Stop is being triggered
                safeSerialPrintf("E-Stop has been triggered!\n");
                pulseValue = MIN_BRIGHTNESS;  // Start pulsing from minimum brightness
                pulseDelta = 1;  // Start increasing brightness
            } else {  // E-Stop is being released
                safeSerialPrintf("E-Stop has been released\n");
                setOutputLEDs(0);  // Set LEDs to full brightness (0 since active LOW)
            }
            
            triggered = currentState;
            lastDebounceTime = currentTime;
        }
    }

    // Update LED pulsing if E-Stop is triggered
    if (triggered) {
        updateLEDPulse();
    }
}
