#include "header/connection.h"

#include <ArduinoJson.h>
#include <AsyncMqttClient.h>

#include "header/variable.h"

AsyncMqttClient mqttClient;

TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

// Function pointer to store the registered event handler
static EventHandler registeredHandler = nullptr;
static LogScreen printLog = nullptr;

bool isConnected = false;

String IPAddToString(IPAddress ip) {  // IP v4 only
    String ips;
    ips.reserve(16);
    ips = ip[0];
    ips += ':';
    ips += ip[1];
    ips += ':';
    ips += ip[2];
    ips += ':';
    ips += ip[3];
    return ips;
}

// Function to register a listener
void registerHandlerListener(EventHandler eventHandler) {
    registeredHandler = eventHandler;
}  // Function to register a listener
void registerLogListener(LogScreen logScreen) { printLog = logScreen; }

void setupAccessPoint() {
    printLog("Starting Access Point");
    // Create an access point
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
    // Print the IP address
    printLog("========================");
    printLog("WEB: http://" + IPAddToString(WiFi.softAPIP()));
    printLog("PASS: " + String(WIFI_AP_PASSWORD));
    printLog("SSID: " + String(WIFI_AP_SSID));
    printLog("========================");
}

void checkWiFiCredentials(const String &selectedSSID, const String &password,
                          bool &isConnected) {
    printLog("Checking WiFi credentials...");
    int attempts = 0;

    WiFi.begin(selectedSSID.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED && attempts < 2) {
        delay(1000);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        printLog("Connected to WiFi");
        isConnected = true;
    } else {
        printLog("Invalid WiFi credentials.");
        isConnected = false;
    }
}

void saveWiFiCredentials(const String &selectedSSID, const String &password) {
    printLog("Saving WiFi credentials..");
    // Clear previous credentials
    for (int i = 0; i < EEPROM_SIZE; ++i) {
        EEPROM.write(i, 0);
    }
    // Write new credentials
    selectedSSID.toCharArray((char *)EEPROM.getDataPtr(),
                             selectedSSID.length() + 1);
    password.toCharArray(
        (char *)EEPROM.getDataPtr() + selectedSSID.length() + 1,
        password.length() + 1);
    // Commit the changes
    EEPROM.commit();
}

bool loadWiFiCredentials(String &ssid, String &password) {
    // Read SSID
    ssid = EEPROM.readString(0);
    if (ssid.length() == 0) {
        return false;
    }
    // Read password
    password = EEPROM.readString(ssid.length() + 1);
    if (password.length() == 0) {
        return false;
    }
    return true;
}

void connectToWifi(const String ssid, const String password) {
    printLog("Connecting to Wi-Fi...");
    WiFi.begin(ssid.c_str(), password.c_str());
}

void connectToSocket() {
    printLog("Connecting to MQTT...");
    mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi-event] event: %d\n", event);
    switch (event) {
        case SYSTEM_EVENT_STA_GOT_IP:
            printLog("WiFi connected.");
            delay(100);
            printLog("IP address: " + IPAddToString(WiFi.localIP()));
            delay(100);
            connectToSocket();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            printLog("WiFi lost connection");
            xTimerStop(mqttReconnectTimer, 0);
            xTimerStart(wifiReconnectTimer, 0);
            break;
    }
}

void onMqttConnect(bool sessionPresent) {
    printLog("Connected to MQTT");
    mqttClient.subscribe(MQTT_TOPIC_RECEIVE_FROM, 0);
    printLog("Communicate with machine...");

    // let machine know
    DynamicJsonDocument jsonDoc(200);
    jsonDoc["command"] = "REQUEST_STATE";
    // Serialize JSON to a string
    String jsonString;
    serializeJson(jsonDoc, jsonString);
    // Convert String to char*
    const char *jsonStringChar = jsonString.c_str();
    // Send JSON over serial
    mqttClient.publish(MQTT_TOPIC_PUBLISH_TO, 0, false, jsonStringChar);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    (void)reason;

    printLog("Disconnected from MQTT.");
    printLog("Reconnecting to MQTT...");

    if (WiFi.isConnected()) {
        xTimerStart(mqttReconnectTimer, 0);
    }
}

void onMqttMessage(char *topic, char *payload,
                   const AsyncMqttClientMessageProperties &properties,
                   const size_t &len, const size_t &index,
                   const size_t &total) {
    String tempPayload = "";
    for (int i = 0; i < len; i++) {
        tempPayload += (char)payload[i];
    }

    if (registeredHandler) {
        registeredHandler(topic, tempPayload);
    }
}

void onMqttPublish(const uint16_t &packetId) {
    printLog("Publish packetId: " + packetId);
}

void setupConnection() {
    WiFi.onEvent(WiFiEvent);

    mqttReconnectTimer = xTimerCreate(
        "mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0,
        reinterpret_cast<TimerCallbackFunction_t>(connectToSocket));
    wifiReconnectTimer =
        xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0,
                     reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);

    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
}