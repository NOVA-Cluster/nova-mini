#include "SimonaDisplay.h"
#include "SimonaTypes.h" // For stage definitions if needed
#include <Arduino.h>
#include "main.h" // Added for safeSerialPrintf

/*
 * displaySimonaStageWaiting
 * Shows that the game is in the WAITING stage, where no player action has been taken yet.
 */
void displaySimonaStageWaiting(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_WAITING\n");
        safeSerialPrintf("Message contents:\n");
        safeSerialPrintf("  message_id: %d\n", msg.message_id);
        safeSerialPrintf("  stage: SIMONA_STAGE_WAITING\n");
        safeSerialPrintf("  level: %d\n", msg.level);
        safeSerialPrintf("  gamePlay: %d\n", msg.gamePlay);
        safeSerialPrintf("  lost: %d\n", msg.lost);
        safeSerialPrintf("  litButton: %d\n", msg.litButton);
        safeSerialPrintf("  lastPressedButton: %d\n", msg.lastPressedButton);
    }
}

/*
 * displaySimonaStageSequenceGeneration
 * Indicates that the game is generating the sequence of moves for the next round.
 */
void displaySimonaStageSequenceGeneration(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_SEQUENCE_GENERATION\n");
        safeSerialPrintf("Message contents:\n");
        safeSerialPrintf("  message_id: %d\n", msg.message_id);
        safeSerialPrintf("  stage: SIMONA_STAGE_SEQUENCE_GENERATION\n");
        safeSerialPrintf("  level: %d\n", msg.level);
        safeSerialPrintf("  gamePlay: %d\n", msg.gamePlay);
        safeSerialPrintf("  lost: %d\n", msg.lost);
        safeSerialPrintf("  litButton: %d\n", msg.litButton);
        safeSerialPrintf("  lastPressedButton: %d\n", msg.lastPressedButton);
    }
}

/*
 * displaySimonaStageTransition
 * Shows the transition stage between the end of one round and the beginning of another.
 */
void displaySimonaStageTransition(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_TRANSITION\n");
        safeSerialPrintf("Message contents:\n");
        safeSerialPrintf("  message_id: %d\n", msg.message_id);
        safeSerialPrintf("  stage: SIMONA_STAGE_TRANSITION\n");
        safeSerialPrintf("  level: %d\n", msg.level);
        safeSerialPrintf("  gamePlay: %d\n", msg.gamePlay);
        safeSerialPrintf("  lost: %d\n", msg.lost);
        safeSerialPrintf("  litButton: %d\n", msg.litButton);
        safeSerialPrintf("  lastPressedButton: %d\n", msg.lastPressedButton);
    }
}

/*
 * displaySimonaStageInputCollection
 * Displays details when waiting for player input during the game.
 */
void displaySimonaStageInputCollection(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_INPUT_COLLECTION\n");
        safeSerialPrintf("Message contents:\n");
        safeSerialPrintf("  message_id: %d\n", msg.message_id);
        safeSerialPrintf("  stage: SIMONA_STAGE_INPUT_COLLECTION\n");
        safeSerialPrintf("  level: %d\n", msg.level);
        safeSerialPrintf("  gamePlay: %d\n", msg.gamePlay);
        safeSerialPrintf("  lost: %d\n", msg.lost);
        safeSerialPrintf("  litButton: %d\n", msg.litButton);
        safeSerialPrintf("  lastPressedButton: %d\n", msg.lastPressedButton);
    }
}

/*
 * displaySimonaStageVerification
 * Indicates that the game is verifying the player's input against the generated sequence.
 */
void displaySimonaStageVerification(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_VERIFICATION\n");
        safeSerialPrintf("Message contents:\n");
        safeSerialPrintf("  message_id: %d\n", msg.message_id);
        safeSerialPrintf("  stage: SIMONA_STAGE_VERIFICATION\n");
        safeSerialPrintf("  level: %d\n", msg.level);
        safeSerialPrintf("  gamePlay: %d\n", msg.gamePlay);
        safeSerialPrintf("  lost: %d\n", msg.lost);
        safeSerialPrintf("  litButton: %d\n", msg.litButton);
        safeSerialPrintf("  lastPressedButton: %d\n", msg.lastPressedButton);
    }
}

/*
 * displaySimonaStageGameLost
 * Called when the player's input does not match and they lose the game.
 */
void displaySimonaStageGameLost(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_GAME_LOST\n");
        safeSerialPrintf("Message contents:\n");
        safeSerialPrintf("  message_id: %d\n", msg.message_id);
        safeSerialPrintf("  stage: SIMONA_STAGE_GAME_LOST\n");
        safeSerialPrintf("  level: %d\n", msg.level);
        safeSerialPrintf("  gamePlay: %d\n", msg.gamePlay);
        safeSerialPrintf("  lost: %d\n", msg.lost);
        safeSerialPrintf("  litButton: %d\n", msg.litButton);
        safeSerialPrintf("  lastPressedButton: %d\n", msg.lastPressedButton);
    }
}

/*
 * displaySimonaStageGameWin
 * Called when the player correctly follows the sequence and wins the round.
 */
void displaySimonaStageGameWin(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_GAME_WIN\n");
        safeSerialPrintf("Message contents:\n");
        safeSerialPrintf("  message_id: %d\n", msg.message_id);
        safeSerialPrintf("  stage: SIMONA_STAGE_GAME_WIN\n");
        safeSerialPrintf("  level: %d\n", msg.level);
        safeSerialPrintf("  gamePlay: %d\n", msg.gamePlay);
        safeSerialPrintf("  lost: %d\n", msg.lost);
        safeSerialPrintf("  litButton: %d\n", msg.litButton);
        safeSerialPrintf("  lastPressedButton: %d\n", msg.lastPressedButton);
    }
}

/*
 * displaySimonaStageReset
 * Indicates that the game is resetting (starting a new game).
 */
void displaySimonaStageReset(const SimonaMessage &msg)
{
    if (1)
    {
        safeSerialPrintf("Stage: SIMONA_STAGE_RESET\n");
        safeSerialPrintf("Message contents:\n");
        safeSerialPrintf("  message_id: %d\n", msg.message_id);
        safeSerialPrintf("  stage: SIMONA_STAGE_RESET\n");
        safeSerialPrintf("  level: %d\n", msg.level);
        safeSerialPrintf("  gamePlay: %d\n", msg.gamePlay);
        safeSerialPrintf("  lost: %d\n", msg.lost);
        safeSerialPrintf("  litButton: %d\n", msg.litButton);
        safeSerialPrintf("  lastPressedButton: %d\n", msg.lastPressedButton);
    }
}
