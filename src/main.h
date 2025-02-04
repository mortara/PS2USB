#include <Arduino.h>
#include "wifimanager.hpp"
#include "webserver.hpp"
#include "ps2devices.h"
#include "usbhost.h"
#include "ArduinoOTA.h"

// Lindy Kabel
// 1: Data -> gelb
// 2: Unused ->Orange
// 3: GND -> grün
// 4: VCC -> rot
// 5: Clock -> schwarz
// 6: Unused ->braun
#define MOUSE_CLK 17
#define MOUSE_DATA 16

#define KEYBOARD_CLK 5
#define KEYBOARD_DATA 4



#define ON LOW
#define OFF HIGH