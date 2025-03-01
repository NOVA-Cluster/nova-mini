#include "utilities/PreferencesManager.h"
#include <Preferences.h>

Preferences PreferencesManager::prefs;
bool PreferencesManager::isInit = false;

void PreferencesManager::init() {
    if (!isInit) {
        prefs.begin(NAMESPACE, false);
        isInit = true;
    }
}

void PreferencesManager::begin() {
    init();
}

void PreferencesManager::end() {
    if (isInit) {
        prefs.end();
        isInit = false;
    }
}

bool PreferencesManager::getBool(const char* key, bool defaultValue) {
    init();
    return prefs.getBool(key, defaultValue);
}

String PreferencesManager::getString(const char* key, String defaultValue) {
    init();
    return prefs.getString(key, defaultValue);
}

int PreferencesManager::getInt(const char* key, int defaultValue) {
    init();
    return prefs.getInt(key, defaultValue);
}

void PreferencesManager::setBool(const char* key, bool value) {
    init();
    prefs.putBool(key, value);
}

void PreferencesManager::setString(const char* key, const String& value) {
    init();
    prefs.putString(key, value);
}

void PreferencesManager::setInt(const char* key, int value) {
    init();
    prefs.putInt(key, value);
}
