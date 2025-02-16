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

void NovaLogo() {
    Serial.println("                                                                           ");
    Serial.println("                                ........                                   ");
    Serial.println("                            ..::--==+++++==--:..                           ");
    Serial.println("                           :-=++++==-:::::::---.                           ");
    Serial.println("                         :=++=-:.:-=+**#####***+-:                         ");
    Serial.println("                      .:++-::-+#%%%#*+==------=+***-.                      ");
    Serial.println("                     .-+::=*%%#+-:.   ...:::::....:=-.                     ");
    Serial.println("                    .:::+%%+-.   :-=*##%%%%@%%%%#*=:..                     ");
    Serial.println("                     .+%#-.  :=*%%%%%#+==------=+*%%#-                     ");
    Serial.println("                    .*%-  :=#@@#*=:.     ......    :+%-                    ");
    Serial.println("                    =#. :+%@#=:.   .-=+*########*+-. :=                    ");
    Serial.println("                    =. =@@*:.  .-+#@@%%##*++++**#%@%=...                   ");
    Serial.println("                    ..+@#:  .-*%@%*+-:.          .-*@+                     ");
    Serial.println("                     :@#.  -#@@*-.   .:=++****+=:. .+#.                    ");
    Serial.println("                     :%: .+@@+.  .:+#%%%%##**#%%@%- ::                     ");
    Serial.println("                     .-:.+@%:  .=#@%*=-.      .:+@%..                      ");
    Serial.println("                      ...*@-  :#@%=:  .-+**#*=. .#*.                       ");
    Serial.println("                         :#-. *@#.  :*%%*+=+@@=.-=.                        ");
    Serial.println("                          .-..+@*.  #@%:..:=#+...                          ");
    Serial.println("                            . .-+=:.:+*+==--. .                            ");
    Serial.println("                                 .... ....                                 ");
    Serial.println("                                                                           ");
    Serial.println("                                                                           ");
    Serial.println("  ...       ...      ..::---:::.        .-.         ::         ..          ");
    Serial.println("  .+=       :+.    .-===-----====:      .**        :#-         =#.         ");
    Serial.println("  .*%=.     -*.  .=*=:..      ..-++.     :%-       +*.        .##+         ");
    Serial.println("  .+=+:     -*.  -+.             .=*:     **.     :%:        .*+.%-        ");
    Serial.println("  .*=..     -*.  ..                -#.    :%-     *+         -#. =#.       ");
    Serial.println("  .*=       -*.                     *=    .*#.   -#.        .#=  .#+       ");
    Serial.println("  .*=       -*.                     =+     :%-  .#=         =*.   :%:      ");
    Serial.println("  .*=       -*.                     *=      *#. =#.        :%:    .+*.     ");
    Serial.println("  .*=     ..-*.                    -#.      :#:.#-        .*+ .:.  .%=     ");
    Serial.println("  .*=     :+=*.                  .=*:       .:.=*.        =#. .#-   -%:    ");
    Serial.println("  .*=     .-%*.      ...     ...-+=.          :#:        .#-   =*.  .**.   ");
    Serial.println("  .+-       =*.      :+=-----==+-.            =+        .+*.   .#-   :#-   ");
    Serial.println("   ..       ...       .::----:.               ..        .:.     :.    ::   ");
    Serial.println("                                                                           ");
}