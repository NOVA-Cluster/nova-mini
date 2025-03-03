#ifndef MIDICONTROL_HPP
#define MIDICONTROL_HPP

#include <MIDI.h>

// Function declarations

void initializeMIDI();
void playGameIntro();
//void playTechnoBassline();
//void playTechnoMelody();
void playBuzzer(int x);
void playLost();
void playWin();
// New declaration for startup music
void playStartupMusic();
void playRoundTransitionMusic(uint8_t round);

#endif // MIDICONTROL_HPP