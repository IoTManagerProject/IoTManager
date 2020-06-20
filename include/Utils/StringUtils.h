#pragma once

#include <Arduino.h>

uint8_t hexStringToUint8(String hex);

uint16_t hexStringToUint16(String hex);

String selectToMarkerLast(String str, String found);

String selectToMarker(String str, String found);

String deleteAfterDelimiter(String str, String found);

String deleteBeforeDelimiter(String str, String found);

String deleteBeforeDelimiterTo(String str, String found);

String selectFromMarkerToMarker(String str, String found, int number);

size_t itemsCount(String str, const String& separator);

boolean isDigitStr(const String&);