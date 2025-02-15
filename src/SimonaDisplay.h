#ifndef SIMONA_H
#define SIMONA_H

#include "SimonaMessage.h"  // Ensure SimonaMessage is declared

// Declare functions to indicate each stage.
void displaySimonaStageWaiting(const SimonaMessage &msg);
void displaySimonaStageSequenceGeneration(const SimonaMessage &msg);
void displaySimonaStageTransition(const SimonaMessage &msg);
void displaySimonaStageInputCollection(const SimonaMessage &msg);
void displaySimonaStageVerification(const SimonaMessage &msg);
void displaySimonaStageGameLost(const SimonaMessage &msg);
void displaySimonaStageGameWin(const SimonaMessage &msg);
void displaySimonaStageReset(const SimonaMessage &msg);

#endif // SIMONA_H
