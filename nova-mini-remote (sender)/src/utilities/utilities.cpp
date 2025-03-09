#include "utilities/utilities.h"
#include <WiFi.h>
#include <stdarg.h>
#include "freertos/semphr.h"
#include <Arduino.h>
#include "../configuration.h"

// Add mutex for thread-safe serial printing
static SemaphoreHandle_t serialMutex;

void initSafeSerial() {
    serialMutex = xSemaphoreCreateMutex();
}

void safeSerialPrintf(const char* format, ...) {
    xSemaphoreTake(serialMutex, portMAX_DELAY);
    char buf[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    Serial.print(buf);
    xSemaphoreGive(serialMutex);
}

// Helper function: printIndent
// Prints 'indent' number of spaces to Serial. This creates a dynamic indent to avoid 
// storing leading space characters in flash memory.
void printIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        Serial.print(' ');
    }
}

// Helper function: printFWithIndent
// First prints an indent using printIndent, then prints a flash-stored string using Serial.println.
// This helps keep the output visually similar while reducing flash memory usage by not storing
// leading whitespace in the string literals.
void printFWithIndent(int indent, const __FlashStringHelper* str) {
    printIndent(indent);
    Serial.println(str);
}

// New structure to hold each logo line's indent value and text.
struct FSLine {
    int indent;
    const __FlashStringHelper* line;
};

// Optimized NovaLogo: uses a table of FSLine entries for reduced code repetition.
void NovaLogo() {
    // Define the logo lines in a static array.
    static const FSLine logoLines[] = {
        {0, F("")},
        {32, F("........")},
        {28, F("..::--==+++++==--:..")},
        {27, F(":-=++++==-:::::::---.")},
        {25, F(":=++=-:.:-=+**#####***+-:")},
        {22, F(".:++-::-+#%%%#*+==------=+***-.")},
        {21, F(".-+::=*%%#+-:.   ...:::::....:=-.")},
        {20, F(":::+%%+-.   :-=*##%%%%@%%%%#*=:..")},
        {21, F(".+%#-.  :=*%%%%%#+==------=+*%%#-")},
        {20, F(".*%-  :=#@@#*=:.     ......    :+%-")},
        {20, F("=#. :+%@#=:.   .-=+*########*+-. :=")},
        {20, F("=. =@@*:.  .-+#@@%%##*++++**#%@%=...")},
        {20, F("..+@#:  .-*%@%*+-:.          .-*@+")},
        {21, F(":@#.  -#@@*-.   .:=++****+=:. .+#.")},
        {21, F(":%: .+@@+.  .:+#%%%%##**#%%@%- ::")},
        {21, F(".-:.+@%:  .=#@%*=-.      .:+@%.")},
        {22, F("...*@-  :#@%=:  .-+**#*=. .#*.")},
        {25, F(":#-. *@#.  :*%%*+=+@@=.-=.")},
        {26, F(".-..+@*.  #@%:..:=#+...")},
        {28, F(". .-+=:.:+*+==--. .")},
        {33, F(".... ....")},
        {0, F("")},
        {0, F("")},
        {2, F("...       ...      ..::---:::.        .-.         ::         ..")},
        {2, F(".+=       :+.    .-===-----====:      .**        :#-         =#.")},
        {2, F(".*%=.     -*.  .=*=:..      ..-++.     :%-       +*.        .##+")},
        {2, F(".+=+:     -*.  -+.             .=*:     **.     :%:        .*+.%-")},
        {2, F(".*=..     -*.  ..                -#.    :%-     *+         -#. =#.")},
        {2, F(".*=       -*.                     *=    .*#.   -#.        .#=  .#+")},
        {2, F(".*=       -*.                     =+     :%-  .#=         =*.   :%:")},
        {2, F(".*=       -*.                     *=      *#. =#.        :%:    .+*.")},
        {2, F(".*=     ..-*.                    -#.      :#:.#-        .*+ .:.  .%=")},
        {2, F(".*=     :+=*.                  .=*:       .:.=*.        =#. .#-   -%:")},
        {2, F(".*=     .-%*.      ...     ...-+=.          :#:        .#-   =*.  .**.")},
        {2, F(".+-       =*.      :+=-----==+-.            =+        .+*.   .#-   :#-")},
        {2, F("..       ...       .::----:.               ..        .:.     :.      -")}
    };

    // Iterate over the array to print each line.
    for (unsigned i = 0; i < sizeof(logoLines) / sizeof(logoLines[0]); i++) {
        printFWithIndent(logoLines[i].indent, logoLines[i].line);
    }
}

String getLastFourOfMac() {
    String mac = WiFi.macAddress();
    String lastPart = mac.substring(mac.length() - 5);
    lastPart.replace(":", "");
    return lastPart;
}

void setLedBrightness(uint8_t led, bool isOn) {
    uint8_t channel;
    switch(led) {
        case BUTTON_RED_OUT: channel = LEDC_CHANNEL_RED; break;
        case BUTTON_GREEN_OUT: channel = LEDC_CHANNEL_GREEN; break;
        case BUTTON_BLUE_OUT: channel = LEDC_CHANNEL_BLUE; break;
        case BUTTON_YELLOW_OUT: channel = LEDC_CHANNEL_YELLOW; break;
        case BUTTON_WHITE_OUT: channel = LEDC_CHANNEL_RESET; break;
        default: return;
    }
    ledcWrite(channel, isOn ? LEDC_FULL_DUTY : LEDC_DIM_DUTY);
}