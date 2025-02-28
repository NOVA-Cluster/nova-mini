#ifndef SIMONAMESSAGE_H
#define SIMONAMESSAGE_H

#include <stdint.h>
#include "SimonaTypes.h"  // Use shared enum definition

// Structure to represent the game state in Simona.
// 'message_id'       : Unique identifier for the message (uint32_t).
// 'stage'            : Current game stage (using SimonaStage from Simona.h).
// 'level'            : Current game level.
// 'gamePlay'         : Counter for tracking the player's sequence inputs.
// 'lost'             : Flag indicating loss (non-zero means game lost).
// 'litButton'        : Used in SIMONA_STAGE_SEQUENCE_GENERATION to indicate which button light is lit.
// 'lastPressedButton': Used in SIMONA_STAGE_INPUT_COLLECTION to indicate the button just pressed.
// 'currentRound'     : Current round number.
// 'maxRounds'        : Maximum rounds.
// 'levelsInRound'    : Levels in current round.
typedef struct {
    uint32_t message_id;         
    SimonaStage stage;
    uint8_t level;
    uint8_t gamePlay;
    uint8_t lost;
    uint8_t litButton;           
    uint8_t lastPressedButton;
    uint8_t currentRound;       // Added: current round number
    uint8_t maxRounds;          // Added: maximum rounds
    uint8_t levelsInRound;      // Added: levels in current round
} SimonaMessage;

#endif // SIMONAMESSAGE_H
