#pragma once

#include <Arduino.h>

void hex2string(byte array[], unsigned int len, char buffer[]);

int string2hex(const char* str, unsigned char* bytes );

uint8_t hexStringToUint8(String hex);

uint16_t hexStringToUint16(String hex);

String selectToMarkerLast(String str, String found);

String selectToMarker(String str, String found);

String extractInner(String str);

String deleteAfterDelimiter(String str, String found);

String deleteBeforeDelimiter(String str, String found);

String deleteBeforeDelimiterTo(String str, String found);

String deleteToMarkerLast(String str, String found);

String selectFromMarkerToMarker(String str, String found, int number);

size_t itemsCount2(String str, const String& separator);

char* stringToChar(String& str);

size_t itemsCount(String& str, const char* delim);

boolean isDigitStr(const String& str);

boolean isDigitDotCommaStr(const String& str);

String prettyBytes(size_t size);

