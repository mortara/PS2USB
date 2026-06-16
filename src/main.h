#pragma once
#include "Arduino.h"
#include "pmCommonLib.hpp"
#include "ps2devices.h"
#include "usbhost.h"
#include "freertos/queue.h"

// PS/2 command queue — used to marshal PS/2 calls from Core 0 (web/MQTT)
// to Core 1 (where the PS/2 library tasks run).
enum class PS2CmdType : uint8_t {
    MOUSE_MOVE,
    MOUSE_PRESS,
    MOUSE_RELEASE,
    MOUSE_CLICK,
    MOUSE_DOUBLE_CLICK,
    SET_MOUSE_TIMING,
    KEY_DOWN,
    KEY_UP,
    TYPE_TEXT,
};

struct PS2Cmd {
    PS2CmdType type;
    union {
        struct { int16_t x; int16_t y; int8_t wheel; } move;
        struct { esp32_ps2dev::PS2Mouse::Button button; } mouseBtn;
        struct { uint16_t clockHalfMicros; uint16_t byteIntervalMicros; } mouseTiming;
        struct { esp32_ps2dev::scancodes::Key key; } key;
        char text[241];
    };
};

extern QueueHandle_t ps2CmdQueue;

// Enqueue a PS/2 command from any core. Non-blocking; drops if queue full.
inline void ps2Enqueue(const PS2Cmd& cmd) {
    xQueueSend(ps2CmdQueue, &cmd, 0);
}


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

bool setup_done = false;
bool mqtt_setup = false;
bool disable_secondary_features = false;

const int SWITCH_PINS[] = { 38, 39, 40, 41 };

TaskHandle_t Task1;
TaskHandle_t Task2;

void DisplayLoopTime();
void Task1code( void * parameter);
void Task2code( void * parameter);
void secondary_loop();
