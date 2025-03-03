#ifndef SIMONA_H
#define SIMONA_H

#include <Arduino.h>
#include "EspNow.h"        // For ESP-NOW message transmission
#include "SimonaTypes.h"   // Shared enum types and message structures
#include <ESPUI.h>         // Web UI components
#include "configuration.h" // Pin definitions and game configuration
#include "Web.h"           // For global game settings like SIMONA_CHEAT_MODE

// External variables defined elsewhere
extern uint16_t levelProgressText; // UI control ID for displaying game progress
extern bool GAME_ENABLED;          // Global flag to enable/disable game input processing

/**
 * @brief Function pointer type for controlling LEDs
 * @param led The LED pin/index to control
 * @param state The desired state (true = ON, false = OFF)
 */
typedef void (*LedControlCallback)(uint8_t led, bool state);

/**
 * @brief Function pointer type for reading button states
 * @param button The button pin/index to read
 * @return The state of the button (true = pressed, false = not pressed)
 */
typedef bool (*ButtonReadCallback)(uint8_t button);

/**
 * @brief Main game controller for the Simona memory game
 *
 * Manages the game state, sequence generation, user input collection,
 * and verification logic for the Simon Says style memory game.
 * Implements a singleton pattern for global access.
 */
class Simona
{
public:
    /**
     * @brief Initialize the singleton instance
     * @param buttons Array of button pin numbers
     * @param leds Array of LED pin numbers
     * @param buttonColors Array of button color names (as strings)
     * @param ledColors Array of LED color names (as strings)
     * @param ledControl Optional callback for LED control
     * @param buttonRead Optional callback for button state reading
     */
    static void initInstance(uint8_t *buttons, uint8_t *leds, const char **buttonColors, const char **ledColors,
                             LedControlCallback ledControl = nullptr, ButtonReadCallback buttonRead = nullptr);

    /**
     * @brief Get the singleton instance
     * @return Pointer to the Simona instance
     */
    static Simona *getInstance() { return instance; }

    /**
     * @brief Load game settings from persistent storage
     */
    void loadPreferences();

    /**
     * @brief Constructor - normally not called directly (use initInstance)
     * @param buttons Array of button pin numbers
     * @param leds Array of LED pin numbers
     * @param buttonColors Array of button color names (as strings)
     * @param ledColors Array of LED color names (as strings)
     * @param ledControl Optional callback for LED control
     * @param buttonRead Optional callback for button state reading
     */
    Simona(uint8_t *buttons, uint8_t *leds, const char **buttonColors, const char **ledColors,
           LedControlCallback ledControl = nullptr, ButtonReadCallback buttonRead = nullptr);

    /**
     * @brief Reset the game to initial state
     */
    void resetGame();

    /**
     * @brief Main game logic task - runs as a FreeRTOS task
     * Handles sequence generation, playback, and game state transitions
     */
    void runGameTask();

    /**
     * @brief Button monitoring task - runs as a FreeRTOS task
     * Monitors the reset button for game reset
     */
    void runButtonTask();

    // Getter methods for game state information

    /**
     * @brief Get the current game level
     * @return Current level number
     */
    uint8_t getCurrentLevel() const { return level; }

    /**
     * @brief Get the number of levels in the current round
     * @return Number of levels
     */
    uint8_t getLevelsInRound() const { return m_levelsInRound; }

    /**
     * @brief Get the current round number
     * @return Current round number
     */
    uint8_t getCurrentRound() const { return m_currentRound; }

    /**
     * @brief Get the maximum number of rounds in the game
     * @return Maximum rounds
     */
    uint8_t getMaxRounds() const { return MAX_ROUNDS; }

    /**
     * @brief Get the current game stage
     * @return Current game stage enum value
     */
    SimonaStage getCurrentStage() const { return stage; }

    /**
     * @brief Get the expected color during input collection
     * @return Color name string or nullptr if not in input collection stage
     */
    const char *getExpectedColor() const
    {
        if (stage == SIMONA_STAGE_INPUT_COLLECTION && game_play <= level)
        {
            return buttonColors[led_simonSaid[game_play]];
        }
        return nullptr;
    }

    /**
     * @brief Get remaining time for input during INPUT_COLLECTION stage
     * @return Seconds remaining or 0 if timer expired or not in input stage
     */
    uint32_t getInputTimeRemaining() const
    {
        if (stage == SIMONA_STAGE_INPUT_COLLECTION)
        {
            uint32_t elapsed = millis() - inputStart;
            if (elapsed < SIMONA_INPUT_TIMEOUT_SECONDS * 1000UL)
            {
                return (SIMONA_INPUT_TIMEOUT_SECONDS * 1000UL - elapsed) / 1000;
            }
        }
        return 0;
    }

    /**
     * @brief Enable or disable cheat mode
     * @param enabled True to enable cheat mode, false to disable
     */
    void setCheatMode(bool enabled) { m_cheatMode = enabled; }

    /**
     * @brief Check if cheat mode is enabled
     * @return True if cheat mode is enabled
     */
    bool getCheatMode() const { return m_cheatMode; }

    /**
     * @brief Enable or disable sequence local echo
     * @param enabled True to enable sequence local echo, false to disable
     */
    void setSequenceLocalEcho(bool enabled) { m_sequenceLocalEcho = enabled; }

    /**
     * @brief Check if sequence local echo is enabled
     * @return True if sequence local echo is enabled
     */
    bool getSequenceLocalEcho() const { return m_sequenceLocalEcho; }

private:
    static Simona *instance; // Singleton instance

    // Game configuration constants
    static const uint8_t MAX_ROUNDS = 4; // Maximum number of rounds to win the game

    // Game state variables
    uint8_t m_levelsInRound = 3; // Number of levels in current round (increases per round)
    uint8_t m_currentRound = 1;  // Current round number

    // Hardware references
    uint8_t *buttons;          // Array of button pins
    uint8_t *leds;             // Array of LED pins
    const char **buttonColors; // Names of button colors (for display)
    const char **ledColors;    // Names of LED colors (for display)

    // Game state tracking
    boolean button[4];          // Current state of each button
    uint8_t bt_simonSaid[100];  // Array to store player's input sequence
    uint8_t led_simonSaid[100]; // Array to store the generated LED sequence
    boolean lost;               // Flag indicating if player has lost
    uint8_t game_play;          // Current position in the sequence during play
    uint8_t level;              // Current level number
    SimonaStage stage;          // Current stage of the game

    // Callback functions
    LedControlCallback ledControl; // Function to control LEDs
    ButtonReadCallback buttonRead; // Function to read button states

    // Game settings
    bool m_cheatMode = false; // Flag for cheat mode (predictable sequence)
    bool m_sequenceLocalEcho; // Flag for sequence local echo
    uint32_t inputStart = 0;  // Timestamp when input collection started (for timeout)

    /**
     * @brief Control an LED's state
     * @param led LED pin/index to control
     * @param state Desired state (true = ON, false = OFF)
     */
    void controlLed(uint8_t led, bool state);

    /**
     * @brief Read a button's state
     * @param button Button pin/index to read
     * @return Button state (true = pressed, false = not pressed)
     */
    bool readButton(uint8_t button);

    /**
     * @brief Update and send a SimonaMessage with current game state
     * @param simMsg Reference to message object to be updated and sent
     */
    void updateAndSendSimMsg(SimonaMessage &simMsg);
};

#endif // SIMONA_H
