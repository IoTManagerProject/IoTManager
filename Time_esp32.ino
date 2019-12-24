#ifdef ESP32

void Time_Init() {

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("[E] Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "[V] %A, %B %d %Y %H:%M:%S");
}

#endif
