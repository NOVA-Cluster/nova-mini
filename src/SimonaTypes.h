#ifndef SIMONATYPES_H
#define SIMONATYPES_H

#include <stdint.h>

enum SimonaStage : uint8_t {
    SIMONA_STAGE_WAITING = 0,
    SIMONA_STAGE_SEQUENCE_GENERATION,
    SIMONA_STAGE_TRANSITION,
    SIMONA_STAGE_INPUT_COLLECTION,
    SIMONA_STAGE_VERIFICATION,
    SIMONA_STAGE_GAME_LOST,
    SIMONA_STAGE_GAME_WIN,
    SIMONA_STAGE_RESET, // Added new stage for reset logic.
    SIMONA_STAGE_ROUND_TRANSITION = 8  // New stage for round transition animation
};

#endif // SIMONATYPES_H
