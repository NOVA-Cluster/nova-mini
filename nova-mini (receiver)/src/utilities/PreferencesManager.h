#ifndef PREFERENCES_MANAGER_H
#define PREFERENCES_MANAGER_H

#include <Preferences.h>
#include <Arduino.h>
#include "../configuration.h"

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

private:
    static Preferences prefs;
    static bool isInit;
    static void init();
};

#endif
