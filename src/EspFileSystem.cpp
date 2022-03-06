#include "EspFileSystem.h"

bool fileSystemInit() {
    if (!FileFS.begin()) {
        SerialPrint('E', "FS", F("Init ERROR, may be FS was not flashed"));
        return false;
    }
    SerialPrint('I', "FS", F("Init completed"));
    return true;
}

void globalVarsSync() {
    settingsFlashJson = readFile(F("settings.json"), 4096);
    settingsFlashJson.replace("\r\n", "");
}

void saveSettingsFlashJson() {
    writeFile(F("settings.json"), settingsFlashJson);
}

void saveParamsFlashJson() {
    writeFile(F("params.json"), paramsFlashJson);
}