#include "SimonaDisplay.h"
#include "SimonaTypes.h" // For stage definitions if needed
#include <Arduino.h>
#include "main.h" // Added to import LEDAnimationState and currentLEDAnimationState
#include "SimonaDisplaySequences.h" // For LED animations

extern int currentLitButton; // Add extern declaration so currentLitButton is visible
extern int currentLastPressedButton; 

/*
 * displaySimonaStageWaiting
 * Shows that the game is in the WAITING stage, where no player action has been taken yet.
 */
void displaySimonaStageWaiting(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_WAITING\nMessage contents:\n  message_id: %d\n  stage: SIMONA_STAGE_WAITING\n  level: %d\n  gamePlay: %d\n  lost: %d\n  litButton: %d\n  lastPressedButton: %d\n",
                         msg.message_id, msg.level, msg.gamePlay, msg.lost, msg.litButton, msg.lastPressedButton);
    }
    currentLEDAnimationState = LED_WAITING;
}

/*
 * displaySimonaStageSequenceGeneration
 * Indicates that the game is generating the sequence of moves for the next round.
 */
void displaySimonaStageSequenceGeneration(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_SEQUENCE_GENERATION\nMessage contents:\n  message_id: %d\n  stage: SIMONA_STAGE_SEQUENCE_GENERATION\n  level: %d\n  gamePlay: %d\n  lost: %d\n  litButton: %d\n  lastPressedButton: %d\n",
                         msg.message_id, msg.level, msg.gamePlay, msg.lost, msg.litButton, msg.lastPressedButton);
    }
    currentLitButton = msg.litButton;
    currentLEDAnimationState = LED_SEQUENCE_GENERATION;
}

/*
 * displaySimonaStageTransition
 * Shows the transition stage between the end of one round and the beginning of another.
 */
void displaySimonaStageTransition(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_TRANSITION\nMessage contents:\n  message_id: %d\n  stage: SIMONA_STAGE_TRANSITION\n  level: %d\n  gamePlay: %d\n  lost: %d\n  litButton: %d\n  lastPressedButton: %d\n",
                         msg.message_id, msg.level, msg.gamePlay, msg.lost, msg.litButton, msg.lastPressedButton);
    }
    currentLEDAnimationState = LED_TRANSITION;
}

/*
 * displaySimonaStageInputCollection
 * Displays details when waiting for player input during the game.
 */
void displaySimonaStageInputCollection(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_INPUT_COLLECTION\nMessage contents:\n  message_id: %d\n  stage: SIMONA_STAGE_INPUT_COLLECTION\n  level: %d\n  gamePlay: %d\n  lost: %d\n  litButton: %d\n  lastPressedButton: %d\n",
                         msg.message_id, msg.level, msg.gamePlay, msg.lost, msg.litButton, msg.lastPressedButton);
    }
    currentLastPressedButton = msg.lastPressedButton;
    currentLEDAnimationState = LED_INPUT_COLLECTION;
}

/*
 * displaySimonaStageVerification
 * Indicates that the game is verifying the player's input against the generated sequence.
 */
void displaySimonaStageVerification(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_VERIFICATION\nMessage contents:\n  message_id: %d\n  stage: SIMONA_STAGE_VERIFICATION\n  level: %d\n  gamePlay: %d\n  lost: %d\n  litButton: %d\n  lastPressedButton: %d\n",
                         msg.message_id, msg.level, msg.gamePlay, msg.lost, msg.litButton, msg.lastPressedButton);
    }
    currentLEDAnimationState = LED_VERIFICATION;
}

/*
 * displaySimonaStageGameLost
 * Called when the player's input does not match and they lose the game.
 */
void displaySimonaStageGameLost(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_GAME_LOST\nMessage contents:\n  message_id: %d\n  stage: SIMONA_STAGE_GAME_LOST\n  level: %d\n  gamePlay: %d\n  lost: %d\n  litButton: %d\n  lastPressedButton: %d\n",
                         msg.message_id, msg.level, msg.gamePlay, msg.lost, msg.litButton, msg.lastPressedButton);
    }
    currentLEDAnimationState = LED_GAME_LOST;
}

/*
 * displaySimonaStageGameWin
 * Called when the player correctly follows the sequence and wins the round.
 */
void displaySimonaStageGameWin(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_GAME_WIN\nMessage contents:\n  message_id: %d\n  stage: SIMONA_STAGE_GAME_WIN\n  level: %d\n  gamePlay: %d\n  lost: %d\n  litButton: %d\n  lastPressedButton: %d\n",
                         msg.message_id, msg.level, msg.gamePlay, msg.lost, msg.litButton, msg.lastPressedButton);
    }
    currentLEDAnimationState = LED_GAME_WIN;
}

/*
 * displaySimonaStageReset
 * Indicates that the game is resetting (starting a new game).
 */
void displaySimonaStageReset(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_RESET\nMessage contents:\n  message_id: %d\n  stage: SIMONA_STAGE_RESET\n  level: %d\n  gamePlay: %d\n  lost: %d\n  litButton: %d\n  lastPressedButton: %d\n",
                         msg.message_id, msg.level, msg.gamePlay, msg.lost, msg.litButton, msg.lastPressedButton);
    }
    currentLEDAnimationState = LED_RESET;
}

/*
 * displaySimonaStageRoundTransition
 * Shows the round transition stage.
 */
void displaySimonaStageRoundTransition(const SimonaMessage &msg)
{
    safeSerialPrintf("Stage: SIMONA_STAGE_ROUND_TRANSITION (message_id: %d, round: %d/%d)\n",
                     msg.message_id, msg.currentRound, msg.maxRounds);
    currentLEDAnimationState = LED_ROUND_TRANSITION; // Set to round transition state!
}
