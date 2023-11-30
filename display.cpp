#include "header/display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Arduino.h>

#include "header/variable.h"

struct POS {
    String x;
    String y;
    String z;
};

struct MACHINE {
    String status;
    String spindle;
    String feedrate;
};

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

MACHINE machine;
POS wPos;
POS mPos;

String movableAxis = "XY";

String logs[MAX_LOG_LINES];
int logCount = 0;
bool isShowSystemLog = true;

void initialize() {
    wPos.x = "0.000";
    wPos.y = "0.000";
    wPos.z = "0.000";
    mPos.x = "0.000";
    mPos.y = "0.000";
    mPos.z = "0.000";
    machine.status = "";
    machine.spindle = "0.00";
    machine.feedrate = "0.00";
}

String textFormat(String st, int maxLength) {
    String newString = "";
    int stLength = st.length();
    int totalLeft = maxLength - stLength;
    for (int i = 0; i < totalLeft; ++i) {
        newString += " ";
    }
    newString += st;
    return newString;
}

void displayCenteredText(String text, int16_t y, int fontSize = 3,
                         int16_t color = ST77XX_WHITE) {
    // Calculate the starting position to center the text
    int textWidth =
        text.length() * 6 * fontSize;  // Assuming a 6-pixel width font
    int startX = (SCREEN_SIZE - textWidth) / 2;
    // Set the cursor to the calculated position and print the text
    tft.setTextSize(fontSize);
    tft.setTextColor(color);
    tft.setCursor(startX + 4, y);
    tft.println(text);
}

void displayCenteredTextOverflow(String text, int fontSize = 3,
                                 int16_t color = ST77XX_WHITE) {
    // Calculate the starting position to center the text horizontally
    int16_t x = (tft.width() - text.length() * 12 * 2) / 2;
    int16_t y = (tft.height() - 16) / 2;
    tft.setCursor(x, y);
    tft.setTextSize(fontSize);
    tft.setTextColor(color);
    tft.print(text);
}

// DISPLAY

void displayMachineStatus(bool isClear = false) {
    const int16_t fontColor = isClear ? ST77XX_BLACK : ST77XX_WHITE;
    displayCenteredText(machine.status, 35, 3, fontColor);
}

void displayXWPOS(bool isClear = false) {
    const int16_t fontColor = isClear ? ST77XX_BLACK : ST77XX_WHITE;
    tft.setTextColor(fontColor);
    tft.setTextSize(2);
    tft.setCursor(40, 100);
    tft.print(textFormat(wPos.x, 7));
}

void displayXMPOS(bool isClear = false) {
    const int16_t fontColor = isClear ? ST77XX_BLACK : ST77XX_WHITE;
    tft.setTextColor(fontColor);
    tft.setTextSize(2);
    tft.setCursor(40, 120);
    tft.print(textFormat(mPos.x, 7));
}

void displayYWPOS(bool isClear = false) {
    const int16_t fontColor = isClear ? ST77XX_BLACK : ST77XX_WHITE;
    tft.setTextColor(fontColor);
    tft.setTextSize(2);
    tft.setCursor(40, 150);
    tft.print(textFormat(wPos.y, 7));
}

void displayYMPOS(bool isClear = false) {
    const int16_t fontColor = isClear ? ST77XX_BLACK : ST77XX_WHITE;
    tft.setTextColor(fontColor);
    tft.setTextSize(2);
    tft.setCursor(40, 170);
    tft.print(textFormat(mPos.y, 7));
}

void displayZWPOS(bool isClear = false) {
    const int16_t fontColor = isClear ? ST77XX_BLACK : ST77XX_WHITE;
    tft.setTextColor(fontColor);
    tft.setTextSize(2);
    tft.setCursor(40, 200);
    tft.print(textFormat(wPos.z, 7));
}

void displayZMPOS(bool isClear = false) {
    const int16_t fontColor = isClear ? ST77XX_BLACK : ST77XX_WHITE;
    tft.setTextColor(fontColor);
    tft.setTextSize(2);
    tft.setCursor(40, 220);
    tft.print(textFormat(mPos.z, 7));
}

void displayPairingLog(bool isClear = false) {
    const int16_t fontColor = isClear ? ST77XX_BLACK : ST77XX_WHITE;
    tft.setTextColor(fontColor);
    tft.setTextSize(2);
    tft.setCursor(40, 220);
    tft.print(textFormat(mPos.z, 7));
}
// END DISPLAY

// SCREEN
void mainScreen(bool isClear = false) {
    const int16_t grey = 0x4a69;
    const int16_t fontColor = isClear ? ST77XX_BLACK : ST77XX_WHITE;
    // BATTERY PERCENTAGE
    tft.setTextColor(grey);
    tft.setTextSize(2);
    tft.setCursor(8, 8);
    tft.print("10%");

    // MACHINE STATUS
    displayMachineStatus();

    // LINE
    tft.drawFastHLine(0, 90, SCREEN_SIZE, grey);
    tft.drawFastVLine(SCREEN_SIZE / 2 + 10, 90, SCREEN_SIZE, grey);
    tft.drawFastHLine(SCREEN_SIZE / 2 + 10, 190, SCREEN_SIZE, grey);

    // X AXIS BOTTOM LEFT
    tft.setTextColor(grey);
    tft.setTextSize(4);
    tft.setCursor(5, 103);
    tft.print("X");
    // X AXIS WPOS
    displayXWPOS();
    // X AXIS MPOS
    displayXMPOS();

    // Y AXIS BUTTOM LEFT
    tft.setTextColor(grey);
    tft.setTextSize(4);
    tft.setCursor(5, 153);
    tft.print("Y");
    // Y AXIS WPOS
    displayYWPOS();
    // Y AXIS MPOS
    displayYMPOS();

    // Z AXIS BUTTOM LEFT
    tft.setTextColor(grey);
    tft.setTextSize(4);
    tft.setCursor(5, 203);
    tft.print("Z");
    // Z AXIS WPOS
    displayZWPOS();
    // Z AXIS MPOS
    displayZMPOS();

    // FEEDRATE & SPINDLE
    tft.setTextSize(2);
    tft.setCursor(127, 200);
    tft.print(textFormat(machine.feedrate + " F", 9));
    tft.setCursor(127, 220);
    tft.print(textFormat(machine.spindle + " S", 9));

    // ACTIVE AXIS
    tft.setTextColor(fontColor);
    tft.setTextSize(4);
    if (movableAxis == "XY") {
        tft.setCursor(172, 115);
    } else {
        tft.setCursor(178, 115);
    }
    tft.print(movableAxis);
    tft.setTextColor(grey);
    tft.setTextSize(2);
    tft.setCursor(165, 160);
    tft.print("Axis");
}

void updateGRBLState(StaticJsonDocument<JSON_DOC_SIZE> json) {
    const char* wPos_x = json["wpos_x"];
    const char* wPos_y = json["wpos_y"];
    const char* wPos_z = json["wpos_z"];
    const char* mPos_x = json["mpos_x"];
    const char* mPos_y = json["mpos_y"];
    const char* mPos_z = json["mpos_z"];
    const char* activeState = json["activeState"];

    if (String(activeState) == "") {
        Serial.println("masuk");
        printLog("Machine port not open.");
    } else {
        Serial.println("gagal masuk");
        if (isShowSystemLog) {
            tft.fillScreen(ST77XX_BLACK);
            isShowSystemLog = false;
            mainScreen();
        }

        if (wPos.x != String(wPos_x)) {
            displayXWPOS(true);
            wPos.x = String(wPos_x);
            displayXWPOS();
        }
        if (mPos.x != String(mPos_x)) {
            displayXMPOS(true);
            mPos.x = String(mPos_x);
            displayXMPOS();
        }

        if (wPos.y != String(wPos_y)) {
            displayYWPOS(true);
            wPos.y = String(wPos_y);
            displayYWPOS();
        }

        if (mPos.y != String(mPos_y)) {
            displayYMPOS(true);
            mPos.y = String(mPos_y);
            displayYMPOS();
        }

        if (wPos.z != String(wPos_z)) {
            displayZWPOS(true);
            wPos.z = String(wPos_z);
            displayZWPOS();
        }

        if (mPos.z != String(mPos_z)) {
            displayZMPOS(true);
            displayZMPOS();
        }

        if (machine.status != String(activeState)) {
            displayMachineStatus(true);
            machine.status = String(activeState);
            displayMachineStatus();
        }

        // TODO:
        machine.spindle = "0.000";
        machine.feedrate = "0.000";
    }
}

void displayLogs() {
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);

    int lineHeight = 10;  // Adjust based on your display size and font size
    int startY = 5;

    // Calculate the maximum number of log entries that can fit on the screen
    int maxLogs = min(logCount, MAX_LOG_LINES);

    // Display the log entries
    for (int i = 0; i < maxLogs; i++) {
        int index = (logCount - i - 1 + MAX_LOG_LINES) % MAX_LOG_LINES;
        String logEntry = logs[index];

        // Draw log entry on the TFT display
        tft.setCursor(5, startY + i * lineHeight);
        tft.fillRect(5, startY + i * lineHeight, tft.width() - 5, lineHeight,
                     ST77XX_BLACK);  // Clear the previous content
        tft.print(logEntry);
    }
}

void saveLog(String logEntry) {
    // Save the log entry in the array
    logs[logCount % MAX_LOG_LINES] = logEntry;
    logCount++;
}

void printLog(String logEntry) {
    // if (!isShowSystemLog) {
    //     tft.fillScreen(ST77XX_BLACK);
    //     isShowSystemLog = true;
    // }
    saveLog("[SYS] " + logEntry);
    if (SHOW_LOG_ON == 1) {
        displayLogs();
    } else if (SHOW_LOG_ON == 2) {
        Serial.print("[SYS] " + logEntry);
    } else if (SHOW_LOG_ON == 0) {
        displayLogs();
        Serial.println("[SYS] " + logEntry);
    }
}

void setupDisplay() {
    // init display
    tft.init(240, 240, SPI_MODE3);
    tft.setRotation(1);
    initialize();
    // Set display brightness to 10% (Prevent blinking)
    pinMode(BACKLIGHT_PIN, OUTPUT);
    analogWrite(BACKLIGHT_PIN, 5);
    tft.fillScreen(ST77XX_BLACK);
    printLog("Booting...");
    delay(1000);
}