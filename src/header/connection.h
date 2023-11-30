#ifndef CONNECTION_H
#define CONNECTION_H

#include <EEPROM.h>
#include <WiFi.h>

#include <functional>

struct WiFiInfo {
    String ssid;
    int32_t rssi;
};

// Maximum number of WiFi networks to store
const int MAX_WIFI_NETWORKS = 10;
extern WiFiClient wifiClient;

void setupConnection();
void connectToWifi(const String ssid, const String password);
void setupAccessPoint();
bool loadWiFiCredentials(String &ssid, String &password);
void checkWiFiCredentials(const String &selectedSSID, const String &password,
                          bool &isConnected);
void saveWiFiCredentials(const String &selectedSSID, const String &password);
// listener
typedef std::function<void(char *topic, String action)> EventHandler;
void registerHandlerListener(EventHandler eventHandler);

typedef std::function<void(String log)> LogScreen;
void registerLogListener(LogScreen logScreen);

#endif