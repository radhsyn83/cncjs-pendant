#include "header/controller.h"

#include "Arduino.h"
#include "ezButton.h"
#include "header/display.h"
#include "header/variable.h"

ezButton joystickMiddleButton(JOYSTICK_SWITCH_PIN);
static JoystickAxisMove joystickMove = nullptr;
static PotentioChange activePotentio = nullptr;

unsigned long lastUpdateTime = 0;

const String axisToString(AXIS_DIRECTION axis) {
    switch (axis) {
        case X_P:
            return "X_POSITIVE";
        case X_N:
            return "X_NEGATIVE";
        case Y_P:
            return "Y_POSITIVE";
        case Y_N:
            return "Y_NEGATIVE";
        case Z_P:
            return "Z_POSITIVE";
        case Z_N:
            return "Z_NEGATIVE";
        default:
            return "UNKNOWN_AXIS";
    }
}

const String potentioToString(POTENTIO potentio) {
    switch (potentio) {
        case FEEDRATE:
            return "FEEDRATE";
        case SPINDLE:
            return "SPINDLE";
        case STEP:
            return "STEP";
        default:
            return "UNKNOWN";
    }
}

void registerJoystickAxisMove(JoystickAxisMove joystickAxisMove) {
    joystickMove = joystickAxisMove;
}
void registerActivePotentio(PotentioChange potentioChange) {
    activePotentio = potentioChange;
}

void toggleAxis() {
    if (movableAxis == "XY") {
        movableAxis = "Z";
    } else {
        movableAxis = "XY";
    }
    Serial.println(movableAxis);
}

void initJoystick() {}

void controllerSetup() { joystickMiddleButton.setDebounceTime(50); }

void controllerLoop() {
    // POTENTIOMETER
    static float feedratePreviousValue = -1.0;
    int feedrateAnalogValue = analogRead(PT_FEEDRATE_PIN);
    // read the input on analog pin GPIO36:
    // Rescale to potentiometer's voltage (from 0V to 3.3V):
    float feedrate = map(feedrateAnalogValue, 0, 4095, 0, PT_FEEDRATE_MAX);
    feedrate = ceil(feedrate / 100.0) * 100.0;
    // JOYSTICK
    joystickMiddleButton.loop();
    // Simulate joystick values (replace with actual joystick readings)
    int xAxisValue = analogRead(JOYSTICK_X_AXIS_PIN);
    int yAxisValue = analogRead(JOYSTICK_Y_AXIS_PIN);
    // Map analog values to the range of -100 to 100
    float mappedX = map(xAxisValue, 0, 4095, -100, 100);
    float mappedY = map(yAxisValue, 0, 4095, -100, 100);

    if (millis() - lastUpdateTime > CONTROLLER_UPDATE_INTERVAL) {
        lastUpdateTime = millis();
        // POTENTIO
        if (feedrate != feedratePreviousValue) {
            feedratePreviousValue = feedrate;
            activePotentio(FEEDRATE, feedrate);
        }

        // JOYSTICK
        if (mappedX > JOYSTICK_BUFFER || mappedX < (JOYSTICK_BUFFER * -1)) {
            if (movableAxis == "XY") {
                joystickMove((mappedX < 0) ? X_N : X_P);
            }
        } else if (mappedY > JOYSTICK_BUFFER ||
                   mappedY < (JOYSTICK_BUFFER * -1)) {
            if (movableAxis == "XY") {
                joystickMove((mappedY < 0) ? Y_P : Y_N);
            } else if (movableAxis == "Z") {
                joystickMove((mappedY < 0) ? Z_P : Z_N);
            }
        }
    }

    if (joystickMiddleButton.isReleased()) {
        toggleAxis();
    }
}
