#include "MIDIControl.hpp"

// Uncomment the following line to use RMTMIDIOutput; comment it to use SoftwareSerial.
#define USE_RMT

#define MIDI_TX_PIN 33 // Set desired TX pin

#ifdef USE_RMT
#include "RMTMIDIOutput.h"
// Global instance for RMTMIDIOutput
RMTMIDIOutput midiSerial;
MIDI_CREATE_INSTANCE(RMTMIDIOutput, midiSerial, MIDI);
#else
#include <SoftwareSerial.h>
SoftwareSerial midiSerial(-1, MIDI_TX_PIN);
MIDI_CREATE_INSTANCE(SoftwareSerial, midiSerial, MIDI);
#endif

void initializeMIDI()
{
    Serial.println("Begin MIDI communication");
#ifdef USE_RMT
    midiSerial.begin(MIDI_TX_PIN); // Initialize RMT output using TX pin.
#else
    midiSerial.begin(31250, SWSERIAL_8N1); // Initialize SoftwareSerial at MIDI baud rate.
#endif
}

// Renamed and updated function to play a game intro melody.
void playGameIntro()
{
    Serial.println("Playing high energy game intro");
    // Change instrument on Channel 1 to Rock Organ (GM program 19)
    MIDI.sendProgramChange(19, 1);
    delay(100); // Allow time for instrument change

    // Short, high-energy sequence with rapid, staccato notes.
    int introNotes[] = {60, 64, 67, 72, 67, 64, 60};       // A compact, energetic motif.
    int introDurations[] = {80, 80, 80, 100, 80, 80, 120}; // Faster note durations.

    int numNotes = sizeof(introNotes) / sizeof(introNotes[0]);
    for (int i = 0; i < numNotes; i++)
    {
        MIDI.sendNoteOn(introNotes[i], 120, 1); // High velocity for energy.
        delay(introDurations[i]);
        MIDI.sendNoteOff(introNotes[i], 0, 1);
        delay(20); // Short gap between notes.
    }
    delay(200); // Brief pause at the end.
}

void playBuzzer(int x)
{
    Serial.print("Playing buzzer note: ");
    Serial.println(x);
    // Set instrument on Channel 1 to Acoustic Grand Piano (GM program 1)
    MIDI.sendProgramChange(1, 1);
    delay(100); // Allow time for instrument change

    MIDI.sendNoteOn(x, 90, 1); // Play note on Channel 1 using the piano
    delay(300);
    MIDI.sendNoteOff(x, 0, 1);
    midiSerial.flush(); // Ensure MIDI messages are sent
    delay(10);          // Small delay to ensure notes do not overlap
}

void playLost()
{
    Serial.println("Playing even sadder sound");
    // Change instrument on Channel 3 to a sad tone (e.g., Church Organ, GM program 20).
    MIDI.sendProgramChange(20, 3);
    delay(100); // Allow time for instrument change

    // Extended descending minor scale for a more mournful effect.
    int lostNotes[] = {64, 62, 60, 59, 57, 55, 53, 52};
    int lostDurations[] = {400, 400, 400, 500, 500, 500, 600, 700}; // Longer durations for increased sadness.
    int numNotes = sizeof(lostNotes) / sizeof(lostNotes[0]);
    for (int i = 0; i < numNotes; i++)
    {
        MIDI.sendNoteOn(lostNotes[i], 80, 3); // Lower velocity for a softer tone.
        delay(lostDurations[i]);
        MIDI.sendNoteOff(lostNotes[i], 0, 3);
        midiSerial.flush(); // Ensure the message is sent.
        delay(80);          // Gap between notes.
    }
}

void playWin()
{
    Serial.println("Playing win celebration sequence using a new instrument");
    // Change instrument on Channel 1. For example, 40 = Violin (instrument numbers may vary)
    MIDI.sendProgramChange(40, 2);
    delay(100); // Allow time for instrument change

    // Creative celebration melody: rising and falling scale.
    // Notes: C5, D5, E5, G5, A5, G5, E5, D5, C5 (MIDI: 72, 74, 76, 79, 81, 79, 76, 74, 72)
    int winNotes[] = {72, 74, 76, 79, 81, 79, 76, 74, 72};
    int winDurations[] = {150, 150, 150, 300, 300, 300, 150, 150, 400};
    int numNotes = 9;
    for (int i = 0; i < numNotes; i++)
    {
        MIDI.sendNoteOn(winNotes[i], 110, 2);
        delay(winDurations[i]);
        MIDI.sendNoteOff(winNotes[i], 0, 2);
        midiSerial.flush();
        delay(100); // gap between notes
    }
}

// Updated function to play short startup music (1.5 seconds with a fun ascending/descending melody).
void playStartupMusic()
{
    Serial.println("Playing startup music");
    // Set instrument on Channel 1 (Electric Piano 1, GM program 5)
    MIDI.sendProgramChange(5, 1);
    delay(50);
    
    // Ascending sequence
    MIDI.sendNoteOn(60, 100, 1);  // C4
    delay(150);
    MIDI.sendNoteOff(60, 0, 1);
    delay(50);
    
    MIDI.sendNoteOn(64, 100, 1);  // E4
    delay(150);
    MIDI.sendNoteOff(64, 0, 1);
    delay(50);
    
    MIDI.sendNoteOn(67, 100, 1);  // G4
    delay(150);
    MIDI.sendNoteOff(67, 0, 1);
    delay(50);
    
    // Descending sequence
    MIDI.sendNoteOn(67, 100, 1);  // G4
    delay(150);
    MIDI.sendNoteOff(67, 0, 1);
    delay(50);
    
    MIDI.sendNoteOn(64, 100, 1);  // E4
    delay(150);
    MIDI.sendNoteOff(64, 0, 1);
    delay(50);
    
    MIDI.sendNoteOn(60, 100, 1);  // C4
    delay(150);
    MIDI.sendNoteOff(60, 0, 1);
    delay(50); // Total duration ~1.5 seconds
}

void playRoundTransitionMusic(uint8_t round) {
    Serial.print("Playing round transition music for round ");
    Serial.println(round);
    
    // Use Trumpet (instrument 57) for a fanfare effect
    MIDI.sendProgramChange(57, 1);
    delay(50);
    
    // Base note for the sequence (C4 = 60)
    const int baseNote = 60;
    
    // Play ascending fanfare based on round number
    for (int i = 0; i < round; i++) {
        // Play chord (root + third + fifth)
        MIDI.sendNoteOn(baseNote + i*2, 100, 1);         // root
        MIDI.sendNoteOn(baseNote + i*2 + 4, 100, 1);     // third
        MIDI.sendNoteOn(baseNote + i*2 + 7, 100, 1);     // fifth
        
        delay(200 - (round * 10));  // Faster for higher rounds
        
        MIDI.sendNoteOff(baseNote + i*2, 0, 1);
        MIDI.sendNoteOff(baseNote + i*2 + 4, 0, 1);
        MIDI.sendNoteOff(baseNote + i*2 + 7, 0, 1);
        delay(50);  // Brief pause between chords
    }
    
    // Final celebratory flourish
    int finalNote = baseNote + 12 + round;  // Gets higher with each round
    MIDI.sendNoteOn(finalNote, 120, 1);
    MIDI.sendNoteOn(finalNote + 4, 120, 1);
    MIDI.sendNoteOn(finalNote + 7, 120, 1);
    delay(300);
    MIDI.sendNoteOff(finalNote, 0, 1);
    MIDI.sendNoteOff(finalNote + 4, 0, 1);
    MIDI.sendNoteOff(finalNote + 7, 0, 1);
}
