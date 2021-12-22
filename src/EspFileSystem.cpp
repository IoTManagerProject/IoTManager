#include "EspFileSystem.h"

bool fileSystemInit() {
    if (!FileFS.begin()) {
        SerialPrint(F("E"), F("FS"), F("Init ERROR, may be FS was not flashed"));
        return false;
    }
    SerialPrint(F("i"), F("FS"), F("Init completed"));
    return true;
}
