#ifndef DISPLAY_H
#define DISPLAY_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <header/variable.h>

#include <functional>

void setupDisplay();
void updateGRBLState(StaticJsonDocument<JSON_DOC_SIZE> json);
void printLog(String logEntry);

#endif