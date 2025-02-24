#include "SimonaDisplaySequences.h"
#include <FastLED.h>
#include <Arduino.h>
#include "Utilities.h" // Added to get declaration for safeSerialPrintf
#include "main.h"      // Added to access LEDAnimationState and currentLEDAnimationState

// Assume extern declaration for LED array from main.cpp
extern CRGB leds[];
// Add fallback in case NUM_LEDS_FOR_TEST is not defined.
#ifndef NUM_LEDS_FOR_TEST
#define NUM_LEDS_FOR_TEST 4
#endif

extern int currentLitButton; // Declare the global variable for use in this file

void displaySimonaStageWaitingAnimation()
{
    const int steps = 10;
    const uint8_t offVal = 8;                                       // 3% of 255 ≈ 8
    const TickType_t tickDelay50 = 50 / steps / portTICK_PERIOD_MS; // delay per interpolation step
    CRGB offWhite = CRGB(offVal, offVal, offVal);

    // Define per-color transitions: each stage: red, green, blue.
    struct ColorTransition
    {
        CRGB base;
        CRGB full;
    } colorTransitions[3] = {
        {CRGB(offVal, 0, 0), CRGB(255, 0, 0)}, // Red
        {CRGB(0, offVal, 0), CRGB(0, 255, 0)}, // Green
        {CRGB(0, 0, offVal), CRGB(0, 0, 255)}  // Blue
    };

    while (currentLEDAnimationState == LED_WAITING)
    {
        // if (currentLEDAnimationState != LED_WAITING) // abort waiting animation if state changes
        //     break;

        // Ensure all LEDs are off (5% white)
        fill_solid(leds, NUM_LEDS_FOR_TEST, offWhite);
        FastLED.show();
        for (int i = 0; i < NUM_LEDS_FOR_TEST; i++)
        {
            // Red, then green, then blue transitions.
            for (int c = 0; c < 3; c++)
            {
                // 1. Transition from offWhite to 5% color.
                for (int step = 0; step <= steps; step++)
                {
                    if (currentLEDAnimationState != LED_WAITING)
                        return;
                    float t = step / (float)steps;
                    leds[i].r = offWhite.r + (colorTransitions[c].base.r - offWhite.r) * t;
                    leds[i].g = offWhite.g + (colorTransitions[c].base.g - offWhite.g) * t;
                    leds[i].b = offWhite.b + (colorTransitions[c].base.b - offWhite.b) * t;
                    FastLED.show();
                    vTaskDelay(tickDelay50);
                }
                // 2. Ramp up LED from 5% to 100% for this color.
                for (int step = 0; step <= steps; step++)
                {
                    if (currentLEDAnimationState != LED_WAITING)
                        return;
                    float t = step / (float)steps;
                    leds[i].r = colorTransitions[c].base.r + (colorTransitions[c].full.r - colorTransitions[c].base.r) * t;
                    leds[i].g = colorTransitions[c].base.g + (colorTransitions[c].full.g - colorTransitions[c].base.g) * t;
                    leds[i].b = colorTransitions[c].base.b + (colorTransitions[c].full.b - colorTransitions[c].base.b) * t;
                    FastLED.show();
                    vTaskDelay(tickDelay50);
                }
                // 3. Hold at 100% color.
                if (currentLEDAnimationState != LED_WAITING)
                    return;
                leds[i] = colorTransitions[c].full;
                FastLED.show();
                vTaskDelay(150 / portTICK_PERIOD_MS);
                // 4. Ramp down back to 5% color.
                for (int step = 0; step <= steps; step++)
                {
                    if (currentLEDAnimationState != LED_WAITING)
                        return;
                    float t = step / (float)steps;
                    leds[i].r = colorTransitions[c].full.r - (colorTransitions[c].full.r - colorTransitions[c].base.r) * t;
                    leds[i].g = colorTransitions[c].full.g - (colorTransitions[c].full.g - colorTransitions[c].base.g) * t;
                    leds[i].b = colorTransitions[c].full.b - (colorTransitions[c].full.b - colorTransitions[c].base.b) * t;
                    FastLED.show();
                    vTaskDelay(tickDelay50);
                }
                // 5. Transition from 5% color back to offWhite.
                for (int step = 0; step <= steps; step++)
                {
                    if (currentLEDAnimationState != LED_WAITING)
                        return;
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

// Updated displaySimonaStageSequenceGenerationAnimation to mimic input collection animation.
void displaySimonaStageSequenceGenerationAnimation()
{
    extern int currentLitButton; // still use currentLitButton for this animation
    int button = currentLitButton;
    
    CRGB targetColor;
    int ledIndex = button;
    switch (button)
    {
    case 0:
        targetColor = CRGB(255, 0, 0);
        break; // red, led 0
    case 1:
        targetColor = CRGB(0, 255, 0);
        break; // green, led 1
    case 2:
        targetColor = CRGB(0, 0, 255);
        break; // blue, led 2
    case 3:
        targetColor = CRGB(255, 255, 0);
        break; // yellow, led 3
    default:
        targetColor = CRGB(255, 0, 0);
        ledIndex = 0;
        break;
    }

    const uint8_t offVal = 8; // 3% of 255 ≈ 8
    CRGB offWhite = CRGB(offVal, offVal, offVal);

    // Turn all LEDs off
    fill_solid(leds, NUM_LEDS_FOR_TEST, offWhite);
    // Turn on only the selected LED with its target color
    if (ledIndex >= 0 && ledIndex < NUM_LEDS_FOR_TEST)
    {
        leds[ledIndex] = targetColor;
        triggerRelay(ledIndex, 100);
    }
    FastLED.show();
    vTaskDelay(50 / portTICK_PERIOD_MS);
    // Optionally, turn off the LED after the delay
    fill_solid(leds, NUM_LEDS_FOR_TEST, offWhite);
    FastLED.show();
}

void displaySimonaStageInputCollectionAnimation()
{
    extern int currentLastPressedButton;   // declare external variable for msg.lastPressedButton
    int button = currentLastPressedButton; // default value if undefined may be 0

    CRGB targetColor;
    // Map the button to the corresponding LED index and color.
    int ledIndex = button;
    switch (button)
    {
    case 0:
        targetColor = CRGB(255, 0, 0);
        break; // red, led 0
    case 1:
        targetColor = CRGB(0, 255, 0);
        break; // green, led 1
    case 2:
        targetColor = CRGB(0, 0, 255);
        break; // blue, led 2
    case 3:
        targetColor = CRGB(255, 255, 0);
        break; // yellow, led 3
    default:
        targetColor = CRGB(255, 0, 0);
        ledIndex = 0;
        break;
    }

    const uint8_t offVal = 8; // 3% of 255 ≈ 8
    CRGB offWhite = CRGB(offVal, offVal, offVal);

    // Turn all LEDs off
    fill_solid(leds, NUM_LEDS_FOR_TEST, offWhite);
    // Turn on only the selected LED with its target color
    if (ledIndex >= 0 && ledIndex < NUM_LEDS_FOR_TEST)
    {
        leds[ledIndex] = targetColor;
        triggerRelay(ledIndex, 100);
    }
    FastLED.show();
    vTaskDelay(50 / portTICK_PERIOD_MS);
    // Optionally, turn off the LED after the delay
    fill_solid(leds, NUM_LEDS_FOR_TEST, offWhite);
    FastLED.show();
}

// New function for LED_TRANSITION state
void displaySimonaStageTransitionAnimation()
{
    const int steps = 20;
    const uint8_t lowVal = 5;   // ~2% brightness of 255
    const uint8_t highVal = 15; // ~6% brightness of 255
    const TickType_t delayMs = 30 / portTICK_PERIOD_MS;

    while (currentLEDAnimationState == LED_TRANSITION)
    {
        // Fade up: 2% -> 6%
        for (int step = 0; step <= steps && currentLEDAnimationState == LED_TRANSITION; step++)
        {
            float t = step / (float)steps;
            uint8_t brightness = lowVal + (highVal - lowVal) * t;
            fill_solid(leds, NUM_LEDS_FOR_TEST, CRGB(brightness, brightness, brightness));
            FastLED.show();
            vTaskDelay(delayMs);
        }

        // Fade down: 6% -> 2%
        for (int step = 0; step <= steps && currentLEDAnimationState == LED_TRANSITION; step++)
        {
            float t = step / (float)steps;
            uint8_t brightness = highVal - (highVal - lowVal) * t;
            fill_solid(leds, NUM_LEDS_FOR_TEST, CRGB(brightness, brightness, brightness));
            FastLED.show();
            vTaskDelay(delayMs);
        }
    }
}

// New function for LED_VERIFICATION state
void displaySimonaStageVerificationAnimation()
{
    // TODO: implement verification animation
}

// New function for LED_GAME_LOST state
void displaySimonaStageGameLostAnimation()
{
    const int steps = 20;
    const uint8_t startVal = 5; // starting at 5% brightness
    const TickType_t delayMs = 30 / portTICK_PERIOD_MS;

    for (int step = 0; step <= steps; step++)
    {
        if (currentLEDAnimationState != LED_GAME_LOST)
            return;
        float t = step / (float)steps;
        uint8_t brightness = startVal - startVal * t;
        fill_solid(leds, NUM_LEDS_FOR_TEST, CRGB(brightness, brightness, brightness));
        FastLED.show();
        vTaskDelay(delayMs);
    }
}

// Updated displaySimonaStageGameWinAnimation to include relay sequence during animation.
void displaySimonaStageGameWinAnimation()
{
    // Configurable total duration in ms (default 2000ms)
    const TickType_t totalDuration = 2000 / portTICK_PERIOD_MS;
    // Number of pulses and steps per pulse for a high-energy effect
    const int pulses = 4;
    const int stepsPerPulse = 10;
    const TickType_t delayPerStep = totalDuration / (pulses * stepsPerPulse);

    // Define win color as purple
    CRGB winColor = CRGB(255, 100, 255);

    int currentRelay = 0;  // Cycle relay index from 0 to 3

    // High-energy win animation: multiple pulses using a sine function for smooth brightness transitions.
    for (int pulse = 0; pulse < pulses; pulse++)
    {
        // Trigger the current relay before starting the pulse
        triggerRelay(currentRelay, 100);
        currentRelay = (currentRelay + 1) % 4;
        
        for (int step = 0; step <= stepsPerPulse; step++)
        {
            float fraction = step / (float)stepsPerPulse;
            // Sine curve for a quick pulse (0->1->0)
            float brightnessFactor = sinf(fraction * 3.14159f);
            // Update all LEDs
            for (int i = 0; i < NUM_LEDS_FOR_TEST; i++)
            {
                leds[i].r = (uint8_t)(winColor.r * brightnessFactor);
                leds[i].g = (uint8_t)(winColor.g * brightnessFactor);
                leds[i].b = (uint8_t)(winColor.b * brightnessFactor);
            }
            FastLED.show();
            vTaskDelay(delayPerStep);
        }
    }
}

// New function for LED_RESET state
void displaySimonaStageResetAnimation()
{
    const int steps = 10;
    const uint8_t offVal = 8; // 3% of 255 ≈ 8
    const TickType_t tickDelay50 = 50 / steps / portTICK_PERIOD_MS;
    const TickType_t totalDuration = 1000 / portTICK_PERIOD_MS; // 250ms
    CRGB offWhite = CRGB(offVal, offVal, offVal);
    struct ColorTransition
    {
        CRGB base;
        CRGB full;
    } colorTransitions[3] = {
        {CRGB(offVal, 0, 0), CRGB(255, 0, 0)},
        {CRGB(0, offVal, 0), CRGB(0, 255, 0)},
        {CRGB(0, 0, offVal), CRGB(0, 0, 255)}};

    TickType_t startTime = xTaskGetTickCount();

    while (currentLEDAnimationState == LED_RESET && (xTaskGetTickCount() - startTime) < totalDuration)
    {
        // Loop through each color transition.
        for (int c = 0; c < 3; c++)
        {
            // 1. Transition from offWhite to 5% color.
            for (int step = 0; step <= steps; step++)
            {
                if ((xTaskGetTickCount() - startTime) >= totalDuration)
                {
                    goto reset_end;
                } // new check
                float t = step / (float)steps;
                CRGB color;
                color.r = offWhite.r + (colorTransitions[c].base.r - offWhite.r) * t;
                color.g = offWhite.g + (colorTransitions[c].base.g - offWhite.g) * t;
                color.b = offWhite.b + (colorTransitions[c].base.b - offWhite.b) * t;
                fill_solid(leds, NUM_LEDS_FOR_TEST, color);
                FastLED.show();
                vTaskDelay(tickDelay50);
            }
            // 2. Ramp up LED from 5% to 100% for this color.
            for (int step = 0; step <= steps; step++)
            {
                if ((xTaskGetTickCount() - startTime) >= totalDuration)
                {
                    goto reset_end;
                }
                float t = step / (float)steps;
                CRGB color;
                color.r = colorTransitions[c].base.r + (colorTransitions[c].full.r - colorTransitions[c].base.r) * t;
                color.g = colorTransitions[c].base.g + (colorTransitions[c].full.g - colorTransitions[c].base.g) * t;
                color.b = colorTransitions[c].base.b + (colorTransitions[c].full.b - colorTransitions[c].base.b) * t;
                fill_solid(leds, NUM_LEDS_FOR_TEST, color);
                FastLED.show();
                vTaskDelay(tickDelay50);
            }
            // 3. Hold at 100% color.
            if ((xTaskGetTickCount() - startTime) >= totalDuration)
            {
                goto reset_end;
            }
            fill_solid(leds, NUM_LEDS_FOR_TEST, colorTransitions[c].full);
            FastLED.show();
            vTaskDelay(150 / portTICK_PERIOD_MS);
            // 4. Ramp down back to 5% color.
            for (int step = 0; step <= steps; step++)
            {
                if ((xTaskGetTickCount() - startTime) >= totalDuration)
                {
                    goto reset_end;
                }
                float t = step / (float)steps;
                CRGB color;
                color.r = colorTransitions[c].full.r - (colorTransitions[c].full.r - colorTransitions[c].base.r) * t;
                color.g = colorTransitions[c].full.g - (colorTransitions[c].full.g - colorTransitions[c].base.g) * t;
                color.b = colorTransitions[c].full.b - (colorTransitions[c].full.b - colorTransitions[c].base.b) * t;
                fill_solid(leds, NUM_LEDS_FOR_TEST, color);
                FastLED.show();
                vTaskDelay(tickDelay50);
            }
            // 5. Transition from 5% color back to offWhite.
            for (int step = 0; step <= steps; step++)
            {
                if ((xTaskGetTickCount() - startTime) >= totalDuration)
                {
                    goto reset_end;
                }
                float t = step / (float)steps;
                CRGB color;
                color.r = colorTransitions[c].base.r + (offWhite.r - colorTransitions[c].base.r) * t;
                color.g = colorTransitions[c].base.g + (offWhite.g - colorTransitions[c].base.g) * t;
                color.b = colorTransitions[c].base.b + (offWhite.b - colorTransitions[c].base.b) * t;
                fill_solid(leds, NUM_LEDS_FOR_TEST, color);
                FastLED.show();
                vTaskDelay(tickDelay50);
            }
        }
    }
reset_end:
    while (currentLEDAnimationState == LED_RESET)
    {
        fill_solid(leds, NUM_LEDS_FOR_TEST, offWhite);
        FastLED.show();
        vTaskDelay(50 / portTICK_PERIOD_MS); // adjust delay as needed
    }
}
