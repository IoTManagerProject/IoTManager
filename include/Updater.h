#pragma once

#include <Arduino.h>

namespace Updater {

const String check();
bool upgrade_fs_image();
bool upgrade_firmware();
}  // namespace Updater