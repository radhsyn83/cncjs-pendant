#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <header/connection.h>
#include <header/controller.h>
#include <header/display.h>
#include <header/variable.h>

AsyncWebServer server(80);
WiFiInfo wifiNetworks[MAX_WIFI_NETWORKS];

void onReceiveHandler(char* topic, String payload) {
    Serial.println("Message received:");
    // Convert the payload to a JSON object
    DynamicJsonDocument doc(1024);  // Adjust the size as needed
    deserializeJson(doc, payload);

    const char* command = doc["command"];

    if (String(command) == "Grbl:state") {
        updateGRBLState(doc);
    } else {
        printLog("Unknown command: " + String(command));
    }
}

void onJoystickAxisMove(AXIS_DIRECTION axis) {
    Serial.println("Move: " + axisToString(axis));
}

void onPotentioChange(POTENTIO potentio, float value) {
    Serial.println("Move: " + potentioToString(potentio) +
                   ", Value: " + String(value));
}

void handleRoot(AsyncWebServerRequest* request, int alert = 0) {
    String content =
        "<html><head><style>"
        "body { font-family: Arial, sans-serif; margin: 20px; "
        "background-color: #333; color: #fff; }"
        "form { max-width: 400px; margin: auto; }"
        "label { display: block; margin-top: 10px; }"
        "select, input { width: 100%; padding: 10px; margin: 5px 0; "
        "background-color: #555; color: #fff; border: 1px solid #777; }"
        "input[type='submit'] { background-color: #4CAF50; color: white; "
        "border: none; padding: 15px 20px; cursor: pointer; }"
        "</style></head><body>"
        "<form action='/' method='post'>"
        "  <label for='ssid'>Select SSID:</label>"
        "  <select name='ssid'>";
    // Populate the select options with the stored WiFi networks
    for (int i = 0; i < MAX_WIFI_NETWORKS && wifiNetworks[i].ssid != ""; ++i) {
        content += "<option value='" + wifiNetworks[i].ssid + "'>" +
                   wifiNetworks[i].ssid + "</option>";
    }
    content +=
        "</select>"
        "  <label for='password'>Password:</label>"
        "  <input type='password' name='password'>"
        "  <input type='submit' value='Save'>"
        "</form>";
    // Alert
    if (alert == 1) {
        content +=
            "<br><center><span class'alert' style='color: red; "
            "margin:10px'>WiFi "
            "credentials "
            "not valid.</span></center>";
    }
    content += "</body></html>";
    request->send(200, "text/html", content);
}

void resetEEPROM() {
    printLog("Resetting EEPROM...");
    // Clear EEPROM
    for (int i = 0; i < EEPROM_SIZE; ++i) {
        EEPROM.write(i, 0);
    }
    // Commit the changes
    EEPROM.commit();
}

void scanningWiFi() {
    printLog("Scanning WiFi...");
    // Scan WiFi networks and store the results
    int numNetworks = WiFi.scanNetworks();
    printLog("Found " + String(numNetworks) + " WiFi networks");
    for (int i = 0; i < min(numNetworks, MAX_WIFI_NETWORKS); ++i) {
        wifiNetworks[i].ssid = WiFi.SSID(i);
        wifiNetworks[i].rssi = WiFi.RSSI(i);
        // Serial.printf("Network %d: %s, RSSI: %d\n", i,
        //               wifiNetworks[i].ssid.c_str(), wifiNetworks[i].rssi);
    }
}

void setup() {
    // init eeprom
    EEPROM.begin(EEPROM_SIZE);
    // init pinout
    // pinMode(JOYSTICK_SWITCH_PIN, INPUT_PULLUP);
    // init serial
    Serial.begin(115200);
    while (!Serial && millis() < 5000) {
        delay(500);
    }
    // Setup display
    setupDisplay();
    // Setup display
    controllerSetup();
    // Register controller Handler
    registerHandlerListener(onReceiveHandler);
    registerActivePotentio(onPotentioChange);
    // Register mqtt handler on new messages
    registerHandlerListener(onReceiveHandler);
    // Register display handler
    registerLogListener(printLog);
    // Checking WiFi Credentials on EEPROM
    String ssid, password;
    if (loadWiFiCredentials(ssid, password)) {
        // Setup Connection
        setupConnection();
        // Connect to wifi
        connectToWifi(ssid, password);
    } else {
        printLog("WiFi not setup");
        // Getting wifi list
        scanningWiFi();
        // Start access point mode
        setupAccessPoint();
        // Route for root / web page
        server.on("/", HTTP_GET,
                  [](AsyncWebServerRequest* request) { handleRoot(request); });

        // Route to handle the form submission
        server.on("/", HTTP_POST, [](AsyncWebServerRequest* request) {
            String selectedSSID = request->arg("ssid");
            String password = request->arg("password");

            bool isConnected = false;
            checkWiFiCredentials(selectedSSID, password, isConnected);
            if (isConnected) {
                request->send(200, "text/plain",
                              "WiFi credentials saved. Restarting...");
                saveWiFiCredentials(selectedSSID, password);
                printLog("Restarting in 3 second...");
                delay(5000);
                ESP.restart();
            } else {
                handleRoot(request, 1);
            }
        });

        // Start the server
        server.begin();
    }
}

void loop() {
    controllerLoop();
    delay(50);
}
