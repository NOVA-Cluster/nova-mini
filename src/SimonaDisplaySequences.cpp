#include "SimonaDisplaySequences.h"
#include <FastLED.h>
#include <Arduino.h>
#include "Utilities.h"  // Added to get declaration for safeSerialPrintf
#include "main.h"  // Added to access LEDAnimationState and currentLEDAnimationState

// Assume extern declaration for LED array from main.cpp
extern CRGB leds[];
// Add fallback in case NUM_LEDS_FOR_TEST is not defined.
#ifndef NUM_LEDS_FOR_TEST
#define NUM_LEDS_FOR_TEST 4
#endif

void displaySimonaStageWaitingAnimation() {
    const int steps = 10;
    const uint8_t offVal = 8;   // 3% of 255 ≈ 8
    const TickType_t tickDelay50 = 50 / steps / portTICK_PERIOD_MS; // delay per interpolation step
    CRGB offWhite = CRGB(offVal, offVal, offVal);

    // Define per-color transitions: each stage: red, green, blue.
    struct ColorTransition {
        CRGB base;
        CRGB full;
    } colorTransitions[3] = {
        { CRGB(offVal, 0, 0), CRGB(255, 0, 0) },   // Red
        { CRGB(0, offVal, 0), CRGB(0, 255, 0) },   // Green
        { CRGB(0, 0, offVal), CRGB(0, 0, 255) }    // Blue
    };

    while (true) {
        if (currentLEDAnimationState != LED_WAITING) // abort waiting animation if state changes
            break;
        // Ensure all LEDs are off (5% white)
        fill_solid(leds, NUM_LEDS_FOR_TEST, offWhite);
        FastLED.show();
        for (int i = 0; i < NUM_LEDS_FOR_TEST; i++) {
            // Red, then green, then blue transitions.
            for (int c = 0; c < 3; c++) {
                // 1. Transition from offWhite to 5% color.
                for (int step = 0; step <= steps; step++) {
                    float t = step / (float)steps;
                    leds[i].r = offWhite.r + (colorTransitions[c].base.r - offWhite.r) * t;
                    leds[i].g = offWhite.g + (colorTransitions[c].base.g - offWhite.g) * t;
                    leds[i].b = offWhite.b + (colorTransitions[c].base.b - offWhite.b) * t;
                    FastLED.show();
                    vTaskDelay(tickDelay50);
                }
                // 2. Ramp up LED from 5% to 100% for this color.
                for (int step = 0; step <= steps; step++) {
                    float t = step / (float)steps;
                    leds[i].r = colorTransitions[c].base.r + (colorTransitions[c].full.r - colorTransitions[c].base.r) * t;
                    leds[i].g = colorTransitions[c].base.g + (colorTransitions[c].full.g - colorTransitions[c].base.g) * t;
                    leds[i].b = colorTransitions[c].base.b + (colorTransitions[c].full.b - colorTransitions[c].base.b) * t;
                    FastLED.show();
                    vTaskDelay(tickDelay50);
                }
                // 3. Hold at 100% color.
                leds[i] = colorTransitions[c].full;
                FastLED.show();
                vTaskDelay(150 / portTICK_PERIOD_MS);
                // 4. Ramp down back to 5% color.
                for (int step = 0; step <= steps; step++) {
                    float t = step / (float)steps;
                    leds[i].r = colorTransitions[c].full.r - (colorTransitions[c].full.r - colorTransitions[c].base.r) * t;
                    leds[i].g = colorTransitions[c].full.g - (colorTransitions[c].full.g - colorTransitions[c].base.g) * t;
                    leds[i].b = colorTransitions[c].full.b - (colorTransitions[c].full.b - colorTransitions[c].base.b) * t;
                    FastLED.show();
                    vTaskDelay(tickDelay50);
                }
                // 5. Transition from 5% color back to offWhite.
                for (int step = 0; step <= steps; step++) {
                    float t = step / (float)steps;
                    leds[i].r = colorTransitions[c].base.r + (offWhite.r - colorTransitions[c].base.r) * t;
                    leds[i].g = colorTransitions[c].base.g + (offWhite.g - colorTransitions[c].base.g) * t;
                    leds[i].b = colorTransitions[c].base.b + (offWhite.b - colorTransitions[c].base.b) * t;
                    FastLED.show();
                    vTaskDelay(tickDelay50);
                }
            }
        }
    }
}

void displaySimonaStageReadyAnimation() {
    safeSerialPrintf("displaySimonaStageReadyAnimation called\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void displaySimonaStagePlayingAnimation() {
    safeSerialPrintf("displaySimonaStagePlayingAnimation called\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void displaySimonaStageGameOverAnimation() {
    safeSerialPrintf("displaySimonaStageGameOverAnimation called\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
