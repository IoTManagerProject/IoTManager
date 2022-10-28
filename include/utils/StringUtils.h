#pragma once

#include "Global.h"

void writeUint8tToString(uint8_t* payload, size_t length, size_t headerLenth, String& payloadStr);

void hex2string(byte array[], unsigned int len, char buffer[]);

int string2hex(const char* str, unsigned char* bytes);

uint8_t hexStringToUint8(const String& hex);

uint16_t hexStringToUint16(const String& hex);

String selectToMarkerLast(String str, const String& found);

String selectToMarker(String str, const String& found);

String extractInner(String str);

String deleteAfterDelimiter(String str, const String& found);

String deleteBeforeDelimiter(String str, const String& found);

String deleteBeforeDelimiterTo(String str, const String& found);

String deleteToMarkerLast(String str, const String& found);

String selectFromMarkerToMarker(String str, const String& found, int number);

size_t itemsCount2(String str, const String& separator);

char* stringToChar(const String& str);

//size_t itemsCount(String& str, const char* delim);

boolean isDigitStr(const String& str);

boolean isDigitDotCommaStr(const String& str);

String prettyBytes(size_t size);

String uint64ToString(uint64_t input, uint8_t base = 10);

void cleanString(String& str);
