#include "Arduino.h"

#ifndef USBHOST_H
#define USBHOST_H

#include "EspUsbHost.h"
#include <PS2Keyboard.hpp>

esp32_ps2dev::scancodes::Key USBKeyCodeToPS2ScanCode(uint8_t keycode);

class MyEspUsbHostClass {
public:
    EspUsbHost usb;

    // Cached device info — populated in onDeviceConnected callback
    uint16_t cachedVid = 0;
    uint16_t cachedPid = 0;
    char cachedManufacturer[64] = {};
    char cachedProduct[64] = {};

    struct KeyActivity {
        volatile unsigned long time = 0;
        volatile uint8_t keycode = 0;
        volatile uint32_t totalPresses = 0;
    } lastKey;

    struct MouseMoveActivity {
        volatile unsigned long time = 0;
        volatile int16_t x = 0;
        volatile int16_t y = 0;
        volatile int16_t wheel = 0;
        volatile uint32_t totalMoves = 0;
    } lastMouseMove;

    struct MouseButtonActivity {
        volatile unsigned long time = 0;
        char description[32] = {};
    } lastMouseButton;

    void init();
    void DisplayInfo();
};

extern MyEspUsbHostClass MyEspUsbHost;
#endif
