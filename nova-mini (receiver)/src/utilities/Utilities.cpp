#include "utilities/Utilities.h"
#include <stdarg.h>
#include "freertos/semphr.h"
#include <Arduino.h> // Added to define __FlashStringHelper and F()
#include <WiFi.h>  // Add this include for WiFi functionality

static SemaphoreHandle_t serialMutex;  // Private static variable

void initSafeSerial() {
    serialMutex = xSemaphoreCreateMutex();
}

void safeSerialPrintf(const char* format, ...)
{
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
        {2, F("..       ...       .::----:.               ..        .:.     :.")}
    };

    // Iterate over the array to print each line.
    for (unsigned i = 0; i < sizeof(logoLines) / sizeof(logoLines[0]); i++) {
        printFWithIndent(logoLines[i].indent, logoLines[i].line);
    }
}

String getLastFourOfMac() {
    String mac = WiFi.macAddress();
    String lastPart = mac.substring(mac.length() - 5);  // Get last 5 chars including colon
    lastPart.replace(":", "");  // Remove the colon
    return lastPart;
}