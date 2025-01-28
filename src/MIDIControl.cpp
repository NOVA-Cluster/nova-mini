#include "MIDIControl.hpp"
#include <SoftwareSerial.h>

#define MIDI_TX_PIN 45                      // Set your desired TX pin
SoftwareSerial midiSerial(-1, MIDI_TX_PIN); // RX pin is not used, so set it to -1

// Create a MIDI instance
MIDI_CREATE_INSTANCE(SoftwareSerial, midiSerial, MIDI);


void initializeMIDI() {
    Serial.println("Begin MIDI communication");
    midiSerial.begin(31250); // Initialize the SoftwareSerial at MIDI baud rate
}

void playTechnoBeat() {
    int kick = 36;        // Bass Drum
    int snare = 38;       // Snare Drum
    int hiHatClosed = 42; // Closed Hi-Hat

    for (int i = 0; i < 4; i++) {
        MIDI.sendNoteOn(kick, 127, 10); // Kick drum
        if (i % 2 == 1) { // Snare on beats 2 and 4
            MIDI.sendNoteOn(snare, 100, 10);
        }
        MIDI.sendNoteOn(hiHatClosed, 80, 10); // Hi-hat on every beat
        delay(250); // Quarter-note spacing (120 BPM)

        // Turn off drum notes
        MIDI.sendNoteOff(kick, 0, 10);
        MIDI.sendNoteOff(snare, 0, 10);
        MIDI.sendNoteOff(hiHatClosed, 0, 10);
    }
}

void playTechnoBassline() {
    int bassline[] = {36, 38, 40, 38}; // Repeating notes (C2, D2, E2, D2)
    int duration = 250; // Eighth-note rhythm

    for (int i = 0; i < 4; i++) {
        MIDI.sendNoteOn(bassline[i], 100, 2); // Play bassline on Channel 2
        delay(duration);
        MIDI.sendNoteOff(bassline[i], 0, 2);
    }
}

void playTechnoMelody() {
    int melody[] = {72, 74, 76, 79}; // Notes (C5, D5, E5, G5)
    int duration = 500; // Half-note rhythm

    for (int i = 0; i < 4; i++) {
        MIDI.sendNoteOn(melody[i], 90, 1); // Play melody on Channel 1
        delay(duration);
        MIDI.sendNoteOff(melody[i], 0, 1);
    }
}