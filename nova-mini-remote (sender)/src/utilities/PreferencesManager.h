#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <Preferences.h>
#include <Arduino.h>

class PreferencesManager {
public:
    static void begin();
    static void end();
    
    // Getters with defaults
    static bool getBool(const char* key, bool defaultValue = false);
    static String getString(const char* key, String defaultValue = "");
    static int getInt(const char* key, int defaultValue = 0);
    
    // Setters
    static void setBool(const char* key, bool value);
    static void setString(const char* key, const String& value);
    static void setInt(const char* key, int value);

    // Keys
    static constexpr const char* NAMESPACE = "nova";
    static constexpr const char* KEY_CHEAT_MODE = "cheat_mode";
    static constexpr const char* KEY_REMOTE_MAC = "remote_mac";
    static constexpr const char* KEY_RECEIVER_MAC = "receiver_mac";
    static constexpr const char* KEY_WIRELESS_ENABLED = "wireless_en"; 
    static constexpr const char* KEY_GAME_ENABLED = "game_en"; // Add new key
    static constexpr const char* KEY_SEQUENCE_LOCAL_ECHO = "seq_echo"; // Add new key

private:
    static Preferences prefs;
    static bool isInit;
    static void init();
};

#endif
