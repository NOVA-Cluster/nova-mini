#include "Utilities.h"
#include <stdarg.h>
#include "freertos/semphr.h"

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

void NovaLogo() {
    printFWithIndent(0, F(""));
    printFWithIndent(32, F("........"));
    printFWithIndent(28, F("..::--==+++++==--:.."));
    printFWithIndent(27, F(":-=++++==-:::::::---."));
    printFWithIndent(25, F(":=++=-:.:-=+**#####***+-:"));
    printFWithIndent(22, F(".:++-::-+#%%%#*+==------=+***-."));
    printFWithIndent(21, F(".-+::=*%%#+-:.   ...:::::....:=-."));
    printFWithIndent(20, F(":::+%%+-.   :-=*##%%%%@%%%%#*=:.."));
    printFWithIndent(21, F(".+%#-.  :=*%%%%%#+==------=+*%%#-"));
    printFWithIndent(20, F(".*%-  :=#@@#*=:.     ......    :+%-"));
    printFWithIndent(20, F("=#. :+%@#=:.   .-=+*########*+-. :="));
    printFWithIndent(20, F("=. =@@*:.  .-+#@@%%##*++++**#%@%=..."));
    printFWithIndent(20, F("..+@#:  .-*%@%*+-:.          .-*@+"));
    printFWithIndent(21, F(":@#.  -#@@*-.   .:=++****+=:. .+#."));
    printFWithIndent(21, F(":%: .+@@+.  .:+#%%%%##**#%%@%- ::"));
    printFWithIndent(21, F(".-:.+@%:  .=#@%*=-.      .:+@%."));
    printFWithIndent(22, F("...*@-  :#@%=:  .-+**#*=. .#*."));
    printFWithIndent(25, F(":#-. *@#.  :*%%*+=+@@=.-=."));
    printFWithIndent(26, F(".-..+@*.  #@%:..:=#+..."));
    printFWithIndent(28, F(". .-+=:.:+*+==--. ."));
    printFWithIndent(33, F(".... ...."));
    printFWithIndent(0, F(""));
    printFWithIndent(0, F(""));
    printFWithIndent(2, F("...       ...      ..::---:::.        .-.         ::         .."));
    printFWithIndent(2, F(".+=       :+.    .-===-----====:      .**        :#-         =#."));
    printFWithIndent(2, F(".*%=.     -*.  .=*=:..      ..-++.     :%-       +*.        .##+"));
    printFWithIndent(2, F(".+=+:     -*.  -+.             .=*:     **.     :%:        .*+.%-"));
    printFWithIndent(2, F(".*=..     -*.  ..                -#.    :%-     *+         -#. =#."));
    printFWithIndent(2, F(".*=       -*.                     *=    .*#.   -#.        .#=  .#+"));
    printFWithIndent(2, F(".*=       -*.                     =+     :%-  .#=         =*.   :%:"));
    printFWithIndent(2, F(".*=       -*.                     *=      *#. =#.        :%:    .+*."));
    printFWithIndent(2, F(".*=     ..-*.                    -#.      :#:.#-        .*+ .:.  .%="));
    printFWithIndent(2, F(".*=     :+=*.                  .=*:       .:.=*.        =#. .#-   -%:"));
    printFWithIndent(2, F(".*=     .-%*.      ...     ...-+=.          :#:        .#-   =*.  .**."));
    printFWithIndent(2, F(".+-       =*.      :+=-----==+-.            =+        .+*.   .#-   :#-"));
    printFWithIndent(2, F("..       ...       .::----:.               ..        .:.     :."));

}