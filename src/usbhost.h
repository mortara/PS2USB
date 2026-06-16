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
        volatile int16_t ps2X = 0;
        volatile int16_t ps2Y = 0;
        volatile int32_t totalX = 0;
        volatile int32_t totalY = 0;
        volatile int32_t totalPs2X = 0;
        volatile int32_t totalPs2Y = 0;
        volatile int32_t totalWheel = 0;
        volatile uint32_t totalMoves = 0;
    } lastMouseMove;

    struct MouseButtonActivity {
        volatile unsigned long time = 0;
        char description[32] = {};
        volatile unsigned long lastClickTime = 0;
        volatile unsigned long lastClickIntervalMs = 0;
        volatile unsigned long lastDoubleClickTime = 0;
        volatile uint32_t totalClicks = 0;
        volatile uint32_t totalDoubleClicks = 0;
        volatile uint8_t lastClickButton = 0xFF;
        volatile uint8_t lastDoubleClickButton = 0xFF;
        char doubleClickDescription[32] = {};
    } lastMouseButton;

    // Ring buffer of raw USB mouse reports for diagnostics
    static const uint8_t MOUSE_RAW_LOG_SIZE = 10;
    static const uint8_t MOUSE_RAW_MAX_BYTES = 8;
    struct MouseRawReport {
        unsigned long time = 0;
        uint8_t rawLen = 0;
        uint8_t rawData[MOUSE_RAW_MAX_BYTES] = {};
        uint8_t rptLen = 0;
        uint8_t rptData[MOUSE_RAW_MAX_BYTES] = {};
        int16_t libX = 0;
        int16_t libY = 0;
        int8_t  libWheel = 0;
        uint8_t libButtons = 0;
        uint8_t decodedButtons = 0;
    };
    MouseRawReport mouseRawLog[MOUSE_RAW_LOG_SIZE];
    volatile uint8_t mouseRawHead = 0;  // next write index (ring buffer)

    // HID report descriptor captured once on device connect
    struct HidDescInfo {
        uint8_t  interfaceNumber = 0;
        uint8_t  subClass = 0;     // filled from onHIDInput
        uint8_t  protocol = 0;     // filled from onHIDInput
        uint16_t length = 0;
        uint8_t  data[512] = {};
        bool     valid = false;
    } hidDesc;

    volatile unsigned long ignoreMouseMoveUntil = 0;
    volatile uint16_t mouseSpeedPercent = 75;
    volatile uint16_t mouseClockHalfMicros = 50;
    volatile uint16_t mouseByteIntervalMicros = 500;

    // Parsed mouse report layout derived from the HID descriptor.
    // Bit offsets are relative to the report payload (after the library strips the report-ID byte).
    struct HidMouseLayout {
        bool     valid = false;
        uint8_t  reportId = 0;       // 0 = device has no report IDs

        uint16_t buttonBitOffset = 0;
        uint8_t  buttonCount = 0;    // number of button bits

        uint16_t xBitOffset = 0;
        uint8_t  xBitSize = 8;
        int32_t  xLogMin = -127;
        int32_t  xLogMax = 127;

        uint16_t yBitOffset = 0;
        uint8_t  yBitSize = 8;
        int32_t  yLogMin = -127;
        int32_t  yLogMax = 127;

        bool     wheelPresent = false;
        uint16_t wheelBitOffset = 0;
        uint8_t  wheelBitSize = 8;
    } mouseLayout;

    void init();
    void DisplayInfo();
    void LoadMouseSettings();
    bool SaveMouseSettings();
    void SetMouseSpeedPercent(uint16_t speedPercent);
    uint16_t GetMouseSpeedPercent();
    void RecordMouseButtonEvent(const char *label, const char *action);
    void RecordMouseClick(uint8_t buttonIndex);
    void RecordMouseDoubleClick(uint8_t buttonIndex, unsigned long intervalMs);
};

extern MyEspUsbHostClass MyEspUsbHost;
#endif
