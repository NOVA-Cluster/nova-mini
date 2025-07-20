#include "SimonaDisplaySequences.h"
#include <FastLED.h>
#include <Arduino.h>
#include "utilities/Utilities.h" // Updated path
#include "main.h"      // Added to access LEDAnimationState and currentLEDAnimationState

// Assume extern declaration for LED array from main.cpp
extern CRGB leds[];
// Add fallback in case NUM_LEDS_FOR_TEST is not defined.
#ifndef NUM_LEDS_FOR_TEST
#define NUM_LEDS_FOR_TEST 4
#endif

extern int currentLitButton;         // Declare the global variable for use in this file
extern int currentLastPressedButton; // Properly declare the external variable here

// Modify other animation functions with similar infinite loops
void displaySimonaStageWaitingAnimation()
{
    const int steps = 10;
    const uint8_t offVal = 8;
    const TickType_t tickDelay50 = 50 / steps / portTICK_PERIOD_MS;
    CRGB offWhite = CRGB(offVal, offVal, offVal);

    // Static state variables to reduce recursion and stack usage
    static int ledIndex = 0;
    static int colorIndex = 0;
    static int stepIndex = 0;
    static int animationPhase = 0;

    struct ColorTransition
    {
        CRGB base;
        CRGB full;
    };

    static const ColorTransition colorTransitions[3] = {
        {CRGB(offVal, 0, 0), CRGB(255, 0, 0)}, // Red
        {CRGB(0, offVal, 0), CRGB(0, 255, 0)}, // Green
        {CRGB(0, 0, offVal), CRGB(0, 0, 255)}  // Blue
    };

    // Reset state if we're not in waiting animation mode
    if (currentLEDAnimationState != LED_WAITING)
    {
        // Reset for next time
        ledIndex = 0;
        colorIndex = 0;
        stepIndex = 0;
        animationPhase = 0;
        return;
    }

    // State machine for different animation phases
    switch (animationPhase)
    {
    case 0: // Initialize
        fill_solid(leds, NUM_LEDS_FOR_TEST, offWhite);
        FastLED.show();
        animationPhase = 1;
        stepIndex = 0; // Reset step index for next phase
        return;

    case 1: // Transition from offWhite to 5% base color
    {
        float t = stepIndex / (float)steps;
        leds[ledIndex].r = offWhite.r + (colorTransitions[colorIndex].base.r - offWhite.r) * t;
        leds[ledIndex].g = offWhite.g + (colorTransitions[colorIndex].base.g - offWhite.g) * t;
        leds[ledIndex].b = offWhite.b + (colorTransitions[colorIndex].base.b - offWhite.b) * t;
        FastLED.show();

        if (++stepIndex > steps)
        {
            stepIndex = 0; // Reset step index for next phase
            animationPhase = 2;
        }
        return;
    }

    case 2: // Ramp up from 5% base color to 100% full color
    {
        float t = stepIndex / (float)steps;
        leds[ledIndex].r = colorTransitions[colorIndex].base.r + (colorTransitions[colorIndex].full.r - colorTransitions[colorIndex].base.r) * t;
        leds[ledIndex].g = colorTransitions[colorIndex].base.g + (colorTransitions[colorIndex].full.g - colorTransitions[colorIndex].base.g) * t;
        leds[ledIndex].b = colorTransitions[colorIndex].base.b + (colorTransitions[colorIndex].full.b - colorTransitions[colorIndex].base.b) * t;
        FastLED.show();

        if (++stepIndex > steps)
        {
            stepIndex = 0; // Reset step index for next phase
            animationPhase = 3;
        }
        return;
    }

    case 3: // Hold at 100% full color
        leds[ledIndex] = colorTransitions[colorIndex].full;
        FastLED.show();
        animationPhase = 4; // Move to next phase immediately
        return;

    case 4: // Ramp down from 100% full color to 5% base color
    {
        float t = stepIndex / (float)steps;
        leds[ledIndex].r = colorTransitions[colorIndex].full.r - (colorTransitions[colorIndex].full.r - colorTransitions[colorIndex].base.r) * t;
        leds[ledIndex].g = colorTransitions[colorIndex].full.g - (colorTransitions[colorIndex].full.g - colorTransitions[colorIndex].base.g) * t;
        leds[ledIndex].b = colorTransitions[colorIndex].full.b - (colorTransitions[colorIndex].full.b - colorTransitions[colorIndex].base.b) * t;
        FastLED.show();

        if (++stepIndex > steps)
        {
            stepIndex = 0; // Reset step index for next phase
            animationPhase = 5;
        }
        return;
    }

    case 5: // Transition from 5% base color back to offWhite
    {
        float t = stepIndex / (float)steps;
        leds[ledIndex].r = colorTransitions[colorIndex].base.r + (offWhite.r - colorTransitions[colorIndex].base.r) * t;
        leds[ledIndex].g = colorTransitions[colorIndex].base.g + (offWhite.g - colorTransitions[colorIndex].base.g) * t;
        leds[ledIndex].b = colorTransitions[colorIndex].base.b + (offWhite.b - colorTransitions[colorIndex].base.b) * t;
        FastLED.show();

        if (++stepIndex > steps)
        {
            stepIndex = 0;      // Reset step index for next phase
            animationPhase = 6; // Move to advance phase
        }
        return;
    }

    case 6: // Advance to next color or LED
        // Move to the next color in the sequence
        if (++colorIndex >= 3)
        {
            colorIndex = 0;
            // Move to the next LED when we've cycled through all colors
            if (++ledIndex >= NUM_LEDS_FOR_TEST)
            {
                ledIndex = 0;
            }
        }
        animationPhase = 1;                  // Back to phase 1 for next color/LED
        vTaskDelay(50 / portTICK_PERIOD_MS); // Small delay between cycles
        return;
    }

    // Add task yielding to prevent watchdog issues
    taskYIELD();
    vTaskDelay(tickDelay50);
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
        // Modified relay mapping: led 0 -> relay 3, led 1 -> relay 2, etc.
        triggerRelay(NUM_LEDS_FOR_TEST - 1 - ledIndex, 25);
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
    extern uint32_t currentLastPressedMessageId; // track last message_id for INPUT_COLLECTION animations
    static uint32_t inputCollectionEnterTime = 0;
    static uint32_t lastPoofMessageId = 0;
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
        // Modified relay mapping: led 0 -> relay 3, led 1 -> relay 2, etc.
        // Poof on new message_id and start timer
        if (currentLastPressedMessageId != lastPoofMessageId) {
            Serial.print("DEBUG: new poof LED ");
            Serial.print(ledIndex);
            Serial.print(", message_id ");
            Serial.println(currentLastPressedMessageId);
            lastPoofMessageId = currentLastPressedMessageId;
            inputCollectionEnterTime = millis();
            triggerRelay(NUM_LEDS_FOR_TEST - 1 - ledIndex, 25);
        } else if (millis() - inputCollectionEnterTime <= 200) {
            Serial.print("DEBUG: timer poof LED ");
            Serial.print(ledIndex);
            Serial.print(", message_id ");
            Serial.println(currentLastPressedMessageId);
            triggerRelay(NUM_LEDS_FOR_TEST - 1 - ledIndex, 25);
        }
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
    const int steps = 20;
    const uint8_t lowVal = 5;   // ~2% brightness of 255
    const uint8_t highVal = 15; // ~6% brightness of 255
    const TickType_t delayMs = 30 / portTICK_PERIOD_MS;
    static bool fadeOut = false;

    while (currentLEDAnimationState == LED_VERIFICATION)
    {
        // Fade up: 2% -> 6%
        for (int step = 0; step <= steps && currentLEDAnimationState == LED_VERIFICATION; step++)
        {
            float t = step / (float)steps;
            uint8_t brightness;
            
            if (!fadeOut)
            {
                // Fade up: 2% -> 6%
                brightness = lowVal + (uint8_t)((highVal - lowVal) * t);
            }
            else
            {
                // Fade down: 6% -> 2%
                brightness = highVal - (uint8_t)((highVal - lowVal) * t);
            }

            fill_solid(leds, NUM_LEDS_FOR_TEST, CRGB(brightness, brightness, brightness));
            FastLED.show();
            vTaskDelay(delayMs);
        }

        // Toggle fade direction
        fadeOut = !fadeOut;
    }
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

    int currentRelay = 0; // Cycle relay index from 0 to 3

    // High-energy win animation: multiple pulses using a sine function for smooth brightness transitions.
    for (int pulse = 0; pulse < pulses; pulse++)
    {
        // Trigger the current relay before starting the pulse
        triggerRelay(currentRelay, 25);
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
        FastLED.show();
        vTaskDelay(50 / portTICK_PERIOD_MS); // adjust delay as needed
    }
}

// New function for round transition - using LED_TRANSITION state instead of LED_ROUND_TRANSITION
// Modified round transition animation function to be stack-friendly
void displaySimonaStageRoundTransitionAnimation()
{
    //safeSerialPrintf("********** Starting round transition animation\n");

    // Reduce local variables to minimize stack usage
    const uint8_t pulseCount = 3;
    const uint8_t dimVal = 8; // 3% brightness

    // Use a single static iteration counter instead of nested loops
    static uint8_t animationStep = 0;
    static uint8_t currentPulse = 0;
    static uint8_t currentLED = 0;

    // Celebration colors - using complementary colors for interesting effect
    static const CRGB celebrationColors[] = {
        CRGB(255, 50, 50), // Red-orange
        CRGB(50, 255, 50), // Green
        CRGB(50, 50, 255), // Blue
        CRGB(255, 255, 50) // Yellow
    };

    // Simple state machine replaces nested loops
    switch (animationStep)
    {
    case 0: // Trigger relays in sequence
        if (currentLED < 4)
        {
            triggerRelay(currentLED, 25);
            currentLED++;
            return;
        }
        else
        {
            currentLED = 0;
            animationStep = 1;
            return;
        }

    case 1: // Fade in LEDs
    {
        float factor = currentLED / 15.0f; // 15 steps
        for (int i = 0; i < NUM_LEDS_FOR_TEST; i++)
        {
            CRGB color = celebrationColors[i];
            leds[i] = CRGB(
                color.r * factor,
                color.g * factor,
                color.b * factor);
        }
        FastLED.show();

        if (++currentLED > 15)
        {
            currentLED = 0;
            animationStep = 2;
        }
        return;
    }

    case 2: // Hold full brightness briefly
        animationStep = 3;
        return;

    case 3: // Fade out LEDs
    {
        float factor = 1.0f - (currentLED / 15.0f);
        for (int i = 0; i < NUM_LEDS_FOR_TEST; i++)
        {
            CRGB color = celebrationColors[i];
            leds[i] = CRGB(
                color.r * factor,
                color.g * factor,
                color.b * factor);
        }
        FastLED.show();

        if (++currentLED > 15)
        {
            currentLED = 0;
            currentPulse++;
            if (currentPulse >= pulseCount)
            {
                animationStep = 4;
            }
            else
            {
                animationStep = 0; // Loop back for next pulse
            }
        }
        return;
    }

    case 4: // Set to dim white and finish
        fill_solid(leds, NUM_LEDS_FOR_TEST, CRGB(dimVal, dimVal, dimVal));
        FastLED.show();

        // Reset state for next call
        animationStep = 0;
        currentPulse = 0;
        currentLED = 0;
        return;
    }
}
