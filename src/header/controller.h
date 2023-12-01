#ifndef JOYSTICK_H
#define JOYSTICK_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <header/variable.h>

#include <functional>

enum AXIS_DIRECTION { X_P, X_N, Y_P, Y_N, Z_P, Z_N };
enum POTENTIO { STEP, FEEDRATE, SPINDLE };

extern AXIS_DIRECTION currentMoveAxis;
extern String movableAxis;

void controllerLoop();
void controllerSetup();
const String axisToString(AXIS_DIRECTION axis);
const String potentioToString(POTENTIO potentio);

typedef std::function<void(AXIS_DIRECTION axis)> JoystickAxisMove;
void registerJoystickAxisMove(JoystickAxisMove joystickAxisMove);

typedef std::function<void(POTENTIO potentio, float value)> PotentioChange;
void registerActivePotentio(PotentioChange potentioChange);

#endif