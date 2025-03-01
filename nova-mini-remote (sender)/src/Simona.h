#ifndef SIMONA_H
#define SIMONA_H

#include <Arduino.h>
#include "EspNow.h"         // Added include for EspNow
#include "SimonaTypes.h"    // New: include shared types
#include <ESPUI.h>          // Added include for ESPUI
#include "configuration.h"  // Include configuration.h for pin definitions
#include "Web.h"            // Add this include for SIMONA_CHEAT_MODE

// Remove all pin definitions - they should be in configuration.h

extern uint16_t levelProgressText;  // Declare as extern to share with other files
extern bool GAME_ENABLED;  // Add this line near the other external declarations

typedef void (*LedControlCallback)(uint8_t led, bool state);
typedef bool (*ButtonReadCallback)(uint8_t button);


class Simona
{
public:
  static void initInstance(uint8_t *buttons, uint8_t *leds, const char **buttonColors, const char **ledColors,
                           LedControlCallback ledControl = nullptr, ButtonReadCallback buttonRead = nullptr);
  static Simona* getInstance() { return instance; }
  
  void loadPreferences();  // New method to load preferences

  Simona(uint8_t *buttons, uint8_t *leds, const char **buttonColors, const char **ledColors, LedControlCallback ledControl = nullptr, ButtonReadCallback buttonRead = nullptr);
  void resetGame();
  void runGameTask();
  void runButtonTask();
  // Removed setCheatMode and getCheatMode methods

  // Add these new getter methods
  uint8_t getCurrentLevel() const { return level; }
  uint8_t getLevelsInRound() const { return m_levelsInRound; }
  uint8_t getCurrentRound() const { return m_currentRound; }
  uint8_t getMaxRounds() const { return MAX_ROUNDS; }
  SimonaStage getCurrentStage() const { return stage; }

  // Add new getter method
  const char* getExpectedColor() const { 
      if (stage == SIMONA_STAGE_INPUT_COLLECTION && game_play <= level) {
          return buttonColors[led_simonSaid[game_play]];
      }
      return nullptr;
  }

  // Add new getter method for remaining input time
  uint32_t getInputTimeRemaining() const {
      if (stage == SIMONA_STAGE_INPUT_COLLECTION) {
          uint32_t elapsed = millis() - inputStart;
          if (elapsed < SIMONA_INPUT_TIMEOUT_SECONDS * 1000UL) {
              return (SIMONA_INPUT_TIMEOUT_SECONDS * 1000UL - elapsed) / 1000;
          }
      }
      return 0;
  }

  void setCheatMode(bool enabled) { m_cheatMode = enabled; }
  bool getCheatMode() const { return m_cheatMode; }

private:
  static Simona* instance;

  static const uint8_t MAX_ROUNDS = 4;  // Maximum number of rounds to win the game
  uint8_t m_levelsInRound = 3;           // Starting with 3 levels, renamed to avoid conflicts
  uint8_t m_currentRound = 1;            // Track the current round
  uint8_t *buttons;
  uint8_t *leds;
  const char **buttonColors;
  const char **ledColors;
  boolean button[4];
  uint8_t bt_simonSaid[100];
  uint8_t led_simonSaid[100];
  boolean lost;
  uint8_t game_play, level;
  SimonaStage stage; // Uses SimonaStage from SimonaTypes.h.
  LedControlCallback ledControl;
  ButtonReadCallback buttonRead;
  bool m_cheatMode = false;  // Add this line
  uint32_t inputStart = 0;  // Add this with other member variables

  void controlLed(uint8_t led, bool state);
  bool readButton(uint8_t button);
  // NEW: Helper method declaration.
  void updateAndSendSimMsg(SimonaMessage &simMsg);
};

#endif // SIMONA_H
