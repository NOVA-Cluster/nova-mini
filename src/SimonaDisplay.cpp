#include "SimonaDisplay.h"
#include "SimonaTypes.h"  // For stage definitions if needed
#include <Arduino.h>

void displaySimonaStageWaiting(const SimonaMessage &msg)
{
    Serial.println("Stage: SIMONA_STAGE_WAITING");
}

void displaySimonaStageSequenceGeneration(const SimonaMessage &msg)
{
    Serial.println("Stage: SIMONA_STAGE_SEQUENCE_GENERATION");
}

void displaySimonaStageTransition(const SimonaMessage &msg)
{
    Serial.println("Stage: SIMONA_STAGE_TRANSITION");
}

void displaySimonaStageInputCollection(const SimonaMessage &msg)
{
    Serial.println("Stage: SIMONA_STAGE_INPUT_COLLECTION");
}

void displaySimonaStageVerification(const SimonaMessage &msg)
{
    Serial.println("Stage: SIMONA_STAGE_VERIFICATION");
}

void displaySimonaStageGameLost(const SimonaMessage &msg)
{
    Serial.println("Stage: SIMONA_STAGE_GAME_LOST");
}

void displaySimonaStageGameWin(const SimonaMessage &msg)
{
    Serial.println("Stage: SIMONA_STAGE_GAME_WIN");
}

void displaySimonaStageReset(const SimonaMessage &msg)
{
    Serial.println("Stage: SIMONA_STAGE_RESET");
}
