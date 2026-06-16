#include "usbhost.h"
#include "ps2devices.h"
#include "pmCommonLib.hpp"

static int32_t mouseScaledXCarry = 0;
static int32_t mouseScaledYCarry = 0;
static const uint16_t DEFAULT_MOUSE_SPEED_PERCENT = 75;
static const uint16_t MIN_MOUSE_SPEED_PERCENT = 25;
static const uint16_t MAX_MOUSE_SPEED_PERCENT = 200;
static const char *MOUSE_SETTINGS_CONFIG_FILE = "/mouse_settings.txt";

esp32_ps2dev::scancodes::Key USBKeyCodeToPS2ScanCode(uint8_t keycode)
{
    switch (keycode)
    {
      case 4:   return esp32_ps2dev::scancodes::Key::K_A;
      case 5:   return esp32_ps2dev::scancodes::Key::K_B;
      case 6:   return esp32_ps2dev::scancodes::Key::K_C;
      case 7:   return esp32_ps2dev::scancodes::Key::K_D;
      case 8:   return esp32_ps2dev::scancodes::Key::K_E;
      case 9:   return esp32_ps2dev::scancodes::Key::K_F;
      case 10:  return esp32_ps2dev::scancodes::Key::K_G;
      case 11:  return esp32_ps2dev::scancodes::Key::K_H;
      case 12:  return esp32_ps2dev::scancodes::Key::K_I;
      case 13:  return esp32_ps2dev::scancodes::Key::K_J;
      case 14:  return esp32_ps2dev::scancodes::Key::K_K;
      case 15:  return esp32_ps2dev::scancodes::Key::K_L;
      case 16:  return esp32_ps2dev::scancodes::Key::K_M;
      case 17:  return esp32_ps2dev::scancodes::Key::K_N;
      case 18:  return esp32_ps2dev::scancodes::Key::K_O;
      case 19:  return esp32_ps2dev::scancodes::Key::K_P;
      case 20:  return esp32_ps2dev::scancodes::Key::K_Q;
      case 21:  return esp32_ps2dev::scancodes::Key::K_R;
      case 22:  return esp32_ps2dev::scancodes::Key::K_S;
      case 23:  return esp32_ps2dev::scancodes::Key::K_T;
      case 24:  return esp32_ps2dev::scancodes::Key::K_U;
      case 25:  return esp32_ps2dev::scancodes::Key::K_V;
      case 26:  return esp32_ps2dev::scancodes::Key::K_W;
      case 27:  return esp32_ps2dev::scancodes::Key::K_X;
      case 28:  return esp32_ps2dev::scancodes::Key::K_Y;
      case 29:  return esp32_ps2dev::scancodes::Key::K_Z;
      case 30:  return esp32_ps2dev::scancodes::Key::K_1;
      case 31:  return esp32_ps2dev::scancodes::Key::K_2;
      case 32:  return esp32_ps2dev::scancodes::Key::K_3;
      case 33:  return esp32_ps2dev::scancodes::Key::K_4;
      case 34:  return esp32_ps2dev::scancodes::Key::K_5;
      case 35:  return esp32_ps2dev::scancodes::Key::K_6;
      case 36:  return esp32_ps2dev::scancodes::Key::K_7;
      case 37:  return esp32_ps2dev::scancodes::Key::K_8;
      case 38:  return esp32_ps2dev::scancodes::Key::K_9;
      case 39:  return esp32_ps2dev::scancodes::Key::K_0;
      case 40:  return esp32_ps2dev::scancodes::Key::K_RETURN;
      case 41:  return esp32_ps2dev::scancodes::Key::K_ESCAPE;
      case 42:  return esp32_ps2dev::scancodes::Key::K_BACKSPACE;
      case 43:  return esp32_ps2dev::scancodes::Key::K_TAB;
      case 44:  return esp32_ps2dev::scancodes::Key::K_SPACE;
      case 45:  return esp32_ps2dev::scancodes::Key::K_MINUS;
      case 46:  return esp32_ps2dev::scancodes::Key::K_EQUALS;
      case 47:  return esp32_ps2dev::scancodes::Key::K_LEFTBRACKET;
      case 48:  return esp32_ps2dev::scancodes::Key::K_RIGHTBRACKET;
      case 49:  return esp32_ps2dev::scancodes::Key::K_BACKSLASH;
      case 51:  return esp32_ps2dev::scancodes::Key::K_SEMICOLON;
      case 54:  return esp32_ps2dev::scancodes::Key::K_COMMA;
      case 55:  return esp32_ps2dev::scancodes::Key::K_PERIOD;
      case 56:  return esp32_ps2dev::scancodes::Key::K_SLASH;
      case 57:  return esp32_ps2dev::scancodes::Key::K_CAPSLOCK;
      case 58:  return esp32_ps2dev::scancodes::Key::K_F1;
      case 59:  return esp32_ps2dev::scancodes::Key::K_F2;
      case 60:  return esp32_ps2dev::scancodes::Key::K_F3;
      case 61:  return esp32_ps2dev::scancodes::Key::K_F4;
      case 62:  return esp32_ps2dev::scancodes::Key::K_F5;
      case 63:  return esp32_ps2dev::scancodes::Key::K_F6;
      case 64:  return esp32_ps2dev::scancodes::Key::K_F7;
      case 65:  return esp32_ps2dev::scancodes::Key::K_F8;
      case 66:  return esp32_ps2dev::scancodes::Key::K_F9;
      case 67:  return esp32_ps2dev::scancodes::Key::K_F10;
      case 68:  return esp32_ps2dev::scancodes::Key::K_F11;
      case 69:  return esp32_ps2dev::scancodes::Key::K_F12;
      case 70:  return esp32_ps2dev::scancodes::Key::K_PRINT;
      case 71:  return esp32_ps2dev::scancodes::Key::K_SCROLLOCK;
      case 72:  return esp32_ps2dev::scancodes::Key::K_PAUSE;
      case 73:  return esp32_ps2dev::scancodes::Key::K_INSERT;
      case 74:  return esp32_ps2dev::scancodes::Key::K_HOME;
      case 75:  return esp32_ps2dev::scancodes::Key::K_PAGEUP;
      case 76:  return esp32_ps2dev::scancodes::Key::K_DELETE;
      case 77:  return esp32_ps2dev::scancodes::Key::K_END;
      case 78:  return esp32_ps2dev::scancodes::Key::K_PAGEDOWN;
      case 79:  return esp32_ps2dev::scancodes::Key::K_RIGHT;
      case 80:  return esp32_ps2dev::scancodes::Key::K_LEFT;
      case 81:  return esp32_ps2dev::scancodes::Key::K_DOWN;
      case 82:  return esp32_ps2dev::scancodes::Key::K_UP;
      case 83:  return esp32_ps2dev::scancodes::Key::K_NUMLOCK;
      case 84:  return esp32_ps2dev::scancodes::Key::K_KP_DIVIDE;
      case 85:  return esp32_ps2dev::scancodes::Key::K_KP_MULTIPLY;
      case 86:  return esp32_ps2dev::scancodes::Key::K_KP_MINUS;
      case 87:  return esp32_ps2dev::scancodes::Key::K_KP_PLUS;
      case 88:  return esp32_ps2dev::scancodes::Key::K_KP_ENTER;
      case 89:  return esp32_ps2dev::scancodes::Key::K_KP1;
      case 90:  return esp32_ps2dev::scancodes::Key::K_KP2;
      case 91:  return esp32_ps2dev::scancodes::Key::K_KP3;
      case 92:  return esp32_ps2dev::scancodes::Key::K_KP4;
      case 93:  return esp32_ps2dev::scancodes::Key::K_KP5;
      case 94:  return esp32_ps2dev::scancodes::Key::K_KP6;
      case 95:  return esp32_ps2dev::scancodes::Key::K_KP7;
      case 96:  return esp32_ps2dev::scancodes::Key::K_KP8;
      case 97:  return esp32_ps2dev::scancodes::Key::K_KP9;
      case 98:  return esp32_ps2dev::scancodes::Key::K_KP0;
      case 99:  return esp32_ps2dev::scancodes::Key::K_KP_PERIOD;
      // Modifier keycodes 0xE0–0xE7
      case 0xE0: return esp32_ps2dev::scancodes::Key::K_LCTRL;
      case 0xE1: return esp32_ps2dev::scancodes::Key::K_LSHIFT;
      case 0xE2: return esp32_ps2dev::scancodes::Key::K_LALT;
      case 0xE4: return esp32_ps2dev::scancodes::Key::K_RCTRL;
      case 0xE5: return esp32_ps2dev::scancodes::Key::K_RSHIFT;
      case 0xE6: return esp32_ps2dev::scancodes::Key::K_RALT;
      default:
        return (esp32_ps2dev::scancodes::Key)(keycode - 3);
    }
}

// Extract a signed integer of 'bitSize' bits from 'data' starting at 'bitOffset'.
// Returns 0 if the field does not fit entirely within dataLen bytes.
static int32_t extractBitsSigned(const uint8_t *data, uint16_t dataLen, uint16_t bitOffset, uint8_t bitSize) {
    if (!data) return 0;
    if (bitSize == 0 || bitSize > 32) return 0;
    uint16_t byteStart   = bitOffset / 8;
    uint8_t  bitShift    = bitOffset % 8;
    uint8_t  bytesNeeded = (bitShift + bitSize + 7) / 8;
    // Reject truncated reports rather than silently returning partial data.
    if ((uint32_t)byteStart + bytesNeeded > dataLen) return 0;
    uint32_t raw = 0;
    for (uint8_t b = 0; b < bytesNeeded; b++)
        raw |= (uint32_t)data[byteStart + b] << (8 * b);
    raw >>= bitShift;
    uint32_t mask = (bitSize == 32) ? 0xFFFFFFFFu : ((1u << bitSize) - 1u);
    raw &= mask;
    if (raw & (1u << (bitSize - 1))) raw |= ~mask;  // sign-extend
    return (int32_t)raw;
}

static bool selectMouseReportPayload(const EspUsbHostMouseEvent &evt,
                                     const MyEspUsbHostClass::HidMouseLayout &layout,
                                     const uint8_t *&payload,
                                     uint16_t &payloadLen) {
    payload = nullptr;
    payloadLen = 0;

    const auto useReportData = [&]() -> bool {
        if (!evt.reportData || evt.reportLength == 0) return false;
        payload = (const uint8_t *)evt.reportData;
        payloadLen = (uint16_t)evt.reportLength;
        return true;
    };

    if (layout.reportId == 0) return useReportData();

    if (evt.rawData && evt.rawLength > 1 && evt.rawData[0] == layout.reportId) {
        payload = (const uint8_t *)evt.rawData + 1;
        payloadLen = (uint16_t)(evt.rawLength - 1);
        return true;
    }

    if (evt.reportData && evt.reportLength > 1 && evt.reportData[0] == layout.reportId) {
        payload = (const uint8_t *)evt.reportData + 1;
        payloadLen = (uint16_t)(evt.reportLength - 1);
        return true;
    }

    return useReportData();
}

static bool looksLikeReportIdMousePacket(const uint8_t *data, uint16_t len, uint8_t parsedButtons) {
    if (!data || len < 5) return false;

    // Some boot-protocol mice still include a report ID. EspUsbHost's boot parser
    // then treats byte 0 as the button mask, so the real button byte at byte 1 is
    // hidden behind a constant "button 2 pressed" value.
    if (data[0] == parsedButtons && data[0] >= 1 && data[0] <= 15) return true;

    return false;
}

static bool extractReportIdPacketButtons(const EspUsbHostMouseEvent &evt, uint8_t &buttons) {
    if (evt.rawData && looksLikeReportIdMousePacket((const uint8_t *)evt.rawData,
                                                    (uint16_t)evt.rawLength,
                                                    evt.buttons)) {
        buttons = ((const uint8_t *)evt.rawData)[1] & 0x07;
        return true;
    }

    if (evt.reportData && looksLikeReportIdMousePacket((const uint8_t *)evt.reportData,
                                                       (uint16_t)evt.reportLength,
                                                       evt.buttons)) {
        buttons = ((const uint8_t *)evt.reportData)[1] & 0x07;
        return true;
    }

    return false;
}

static bool decodeReportIdMousePacket(const uint8_t *data,
                                      uint16_t len,
                                      int16_t &x,
                                      int16_t &y,
                                      int8_t &wheel,
                                      uint8_t &buttons) {
    if (!looksLikeReportIdMousePacket(data, len, data ? data[0] : 0)) return false;

    const auto &layout = MyEspUsbHost.mouseLayout;
    const uint8_t *payload = data + 1;
    uint16_t payloadLen = len - 1;

    buttons = data[1] & 0x07;
    x = (payloadLen > 2) ? (int8_t)payload[2] : 0;
    y = (payloadLen > 3) ? (int8_t)payload[3] : 0;
    wheel = (payloadLen > 4) ? (int8_t)payload[4] : 0;

    if (layout.valid && layout.reportId == data[0]) {
        x = (int16_t)extractBitsSigned(payload, payloadLen, layout.xBitOffset, layout.xBitSize);
        y = (int16_t)extractBitsSigned(payload, payloadLen, layout.yBitOffset, layout.yBitSize);
        if (layout.wheelPresent)
            wheel = (int8_t)constrain(extractBitsSigned(payload, payloadLen, layout.wheelBitOffset, layout.wheelBitSize), -128, 127);
        else
            wheel = 0;

        buttons = 0;
        for (uint8_t b = 0; b < layout.buttonCount && b < 8; b++) {
            if (extractBitsSigned(payload, payloadLen, layout.buttonBitOffset + b, 1) & 1)
                buttons |= (uint8_t)(1u << b);
        }
        buttons &= 0x07;
    }

    return true;
}

static void storeMouseRawReport(const uint8_t *rawData,
                                size_t rawLength,
                                const uint8_t *reportData,
                                size_t reportLength,
                                int16_t libX,
                                int16_t libY,
                                int8_t libWheel,
                                uint8_t libButtons,
                                uint8_t decodedButtons) {
    uint8_t idx = MyEspUsbHost.mouseRawHead;
    auto &entry = MyEspUsbHost.mouseRawLog[idx];
    entry.time       = millis();
    entry.rawLen     = (uint8_t)min(rawLength,  (size_t)MyEspUsbHostClass::MOUSE_RAW_MAX_BYTES);
    entry.rptLen     = (uint8_t)min(reportLength,(size_t)MyEspUsbHostClass::MOUSE_RAW_MAX_BYTES);
    if (rawData && entry.rawLen) memcpy(entry.rawData, rawData, entry.rawLen);
    if (reportData && entry.rptLen) memcpy(entry.rptData, reportData, entry.rptLen);
    entry.libX       = libX;
    entry.libY       = libY;
    entry.libWheel   = libWheel;
    entry.libButtons = libButtons;
    entry.decodedButtons = decodedButtons;
    MyEspUsbHost.mouseRawHead = (idx + 1) % MyEspUsbHostClass::MOUSE_RAW_LOG_SIZE;
}

static void handleDecodedMouseButtons(uint8_t previousButtons, uint8_t buttons) {
    buttons &= 0x07;
    previousButtons &= 0x07;
    if (buttons == previousButtons) return;

    uint8_t changed = buttons ^ previousButtons;
    if (changed & 0x01) {
        if (buttons & 0x01) {
            pmLogging.LogLn("Mouse LEFT pressed");
            MyEspUsbHost.RecordMouseButtonEvent("Left", "pressed");
            PS2Devices.PressMouseButton(esp32_ps2dev::PS2Mouse::Button::LEFT);
        } else {
            pmLogging.LogLn("Mouse LEFT released");
            MyEspUsbHost.RecordMouseButtonEvent("Left", "released");
            MyEspUsbHost.RecordMouseClick(0);
            PS2Devices.ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button::LEFT);
        }
    }
    if (changed & 0x02) {
        if (buttons & 0x02) {
            pmLogging.LogLn("Mouse RIGHT pressed");
            MyEspUsbHost.RecordMouseButtonEvent("Right", "pressed");
            PS2Devices.PressMouseButton(esp32_ps2dev::PS2Mouse::Button::RIGHT);
        } else {
            pmLogging.LogLn("Mouse RIGHT released");
            MyEspUsbHost.RecordMouseButtonEvent("Right", "released");
            MyEspUsbHost.RecordMouseClick(1);
            PS2Devices.ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button::RIGHT);
        }
    }
    if (changed & 0x04) {
        if (buttons & 0x04) {
            pmLogging.LogLn("Mouse MIDDLE pressed");
            MyEspUsbHost.RecordMouseButtonEvent("Middle", "pressed");
            PS2Devices.PressMouseButton(esp32_ps2dev::PS2Mouse::Button::MIDDLE);
        } else {
            pmLogging.LogLn("Mouse MIDDLE released");
            MyEspUsbHost.RecordMouseButtonEvent("Middle", "released");
            MyEspUsbHost.RecordMouseClick(2);
            PS2Devices.ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button::MIDDLE);
        }
    }
}

static void handleDecodedMouseMove(int16_t x, int16_t y, int8_t wheel) {
    if (x == 0 && y == 0 && wheel == 0) return;
    if (millis() < MyEspUsbHost.ignoreMouseMoveUntil) return;

    uint16_t speedPercent = MyEspUsbHost.GetMouseSpeedPercent();
    mouseScaledXCarry += (int32_t)x * speedPercent;
    mouseScaledYCarry += (int32_t)y * speedPercent;
    int16_t scaledX = (int16_t)(mouseScaledXCarry / 100);
    int16_t scaledY = (int16_t)(mouseScaledYCarry / 100);
    mouseScaledXCarry -= (int32_t)scaledX * 100;
    mouseScaledYCarry -= (int32_t)scaledY * 100;

    // Clamp to ±127 before handing to the PS/2 library.
    // The PS/2 protocol sets overflow bits when |value| > 255, and many
    // retro PS/2 host drivers react to overflow with wild cursor jumps or
    // a mouse reset. ±127 also matches what the old boot-protocol int8_t
    // gave us, and survives the PS/2 host's optional 2:1 scale mode
    // (127 × 2 = 254, safely below the 255 overflow threshold).
    auto clamp7 = [](int16_t v) -> int16_t {
        return v > 127 ? 127 : (v < -127 ? -127 : v);
    };
    int16_t ps2X = clamp7(scaledX);
    int16_t ps2Y = clamp7((int16_t)-scaledY);
    if (ps2X != 0 || ps2Y != 0 || wheel != 0) {
        PS2Devices.MoveMouse(ps2X, ps2Y, wheel);
    }
    MyEspUsbHost.lastMouseMove.time = millis();
    MyEspUsbHost.lastMouseMove.x = x;
    MyEspUsbHost.lastMouseMove.y = y;
    MyEspUsbHost.lastMouseMove.wheel = wheel;
    MyEspUsbHost.lastMouseMove.ps2X = ps2X;
    MyEspUsbHost.lastMouseMove.ps2Y = ps2Y;
    MyEspUsbHost.lastMouseMove.totalX += x;
    MyEspUsbHost.lastMouseMove.totalY += y;
    MyEspUsbHost.lastMouseMove.totalPs2X += ps2X;
    MyEspUsbHost.lastMouseMove.totalPs2Y += ps2Y;
    MyEspUsbHost.lastMouseMove.totalWheel += wheel;
    MyEspUsbHost.lastMouseMove.totalMoves++;
}

// Walk a raw HID report descriptor and extract the mouse report layout.
// Returns true if X and Y axes were found.
static bool parseHIDMouseLayout(const uint8_t *desc, uint16_t len,
                                MyEspUsbHostClass::HidMouseLayout &out) {
    uint8_t  usagePage = 0;
    int32_t  logMin = 0, logMax = 0;
    uint8_t  reportSize = 0, reportCount = 0;
    uint8_t  reportId = 0;

    // Per-report-ID accumulated bit offset.
    // Index 0 is for reportId==0 (no report IDs used); IDs 1-255 are stored at index 1-255.
    // We cap at 256 entries to cover the full uint8_t range without aliasing.
    uint16_t bitOffsets[256] = {};

    uint8_t  pendingUsages[16];
    uint8_t  pendingUsageCnt = 0;
    uint8_t  pendingUsageMin = 0, pendingUsageMax = 0;
    bool     hasPendingRange = false;

    int      collectionDepth = 0;
    bool     inMouseColl = false;
    int      mouseCollDepth = -1;

    bool     foundX = false, foundY = false;
    uint8_t  mouseReportId = 0;
    uint16_t buttonBitOff = 0; uint8_t  buttonCnt = 0;
    uint16_t xBitOff = 0;     uint8_t  xBits = 8;  int32_t xMin = -127, xMax = 127;
    uint16_t yBitOff = 0;     uint8_t  yBits = 8;  int32_t yMin = -127, yMax = 127;
    bool     hasWheel = false;
    uint16_t wBitOff = 0;     uint8_t  wBits = 8;

    uint16_t i = 0;
    while (i < len) {
        uint8_t prefix    = desc[i++];
        uint8_t bSize     = prefix & 0x03;
        uint8_t bType     = (prefix >> 2) & 0x03;
        uint8_t bTag      = (prefix >> 4) & 0x0F;
        uint8_t dataBytes = (bSize == 3) ? 4 : bSize;
        if (i + dataBytes > len) break;

        uint32_t uval = 0;
        for (uint8_t b = 0; b < dataBytes; b++) uval |= (uint32_t)desc[i + b] << (8 * b);
        int32_t sval = 0;
        if      (dataBytes == 1) sval = (int8_t)(uval & 0xFF);
        else if (dataBytes == 2) sval = (int16_t)(uval & 0xFFFF);
        else                     sval = (int32_t)uval;
        i += dataBytes;

        if (bType == 1) { // Global
            switch (bTag) {
                case 0: usagePage   = (uint8_t)uval; break;
                case 1: logMin      = sval;           break;
                case 2: logMax      = sval;           break;
                case 7: reportSize  = (uint8_t)uval;  break;
                case 8: reportId    = (uint8_t)uval;  break;
                case 9: reportCount = (uint8_t)uval;  break;
            }
        } else if (bType == 2) { // Local
            if (bTag == 0 && pendingUsageCnt < 16) { // Usage
                pendingUsages[pendingUsageCnt++] = (uint8_t)uval;
            } else if (bTag == 1) { pendingUsageMin = (uint8_t)uval; hasPendingRange = true; }
            else if   (bTag == 2) { pendingUsageMax = (uint8_t)uval; hasPendingRange = true; }
        } else if (bType == 0) { // Main
            if (bTag == 10) { // Collection
                collectionDepth++;
                // Detect Mouse application collection (Usage Page 0x01, Usage 0x02)
                if (!inMouseColl && usagePage == 0x01 &&
                    ((pendingUsageCnt > 0 && (pendingUsages[0] == 0x02 || pendingUsages[0] == 0x01)))) {
                    inMouseColl  = true;
                    mouseCollDepth = collectionDepth;
                }
            } else if (bTag == 12) { // End Collection
                if (inMouseColl && collectionDepth == mouseCollDepth) {
                    inMouseColl  = false;
                    mouseCollDepth = -1;
                }
                collectionDepth--;
            } else if (bTag == 8) { // Input
                uint16_t &bitOff = bitOffsets[reportId];

                bool isConstant = (uval & 0x01) != 0;
                if (inMouseColl && !isConstant) {
                    if (usagePage == 0x09) {
                        // Button page — record start bit and accumulate count
                        if (buttonCnt == 0) { buttonBitOff = bitOff; mouseReportId = reportId; }
                        buttonCnt += reportCount;
                    } else if (usagePage == 0x01) {
                        // Generic Desktop axes
                        for (uint8_t slot = 0; slot < reportCount; slot++) {
                            uint8_t usage = 0;
                            if (pendingUsageCnt > 0 && slot < pendingUsageCnt) {
                                usage = pendingUsages[slot];
                            } else if (hasPendingRange && pendingUsageMin + slot <= pendingUsageMax) {
                                usage = pendingUsageMin + slot;
                            }
                            uint16_t fieldOff = bitOff + (uint16_t)slot * reportSize;
                            if (usage == 0x30) {
                                // Starting a new X — reset Y/Wheel found flags so we don't
                                // mix fields from a previous report ID.
                                if (foundX && mouseReportId != reportId) { foundY=false; hasWheel=false; }
                                xBitOff=fieldOff; xBits=reportSize; xMin=logMin; xMax=logMax;
                                foundX=true; mouseReportId=reportId;
                            } else if (usage == 0x31) {
                                // Only accept Y from the same report as X.
                                if (foundX && mouseReportId == reportId) {
                                    yBitOff=fieldOff; yBits=reportSize; yMin=logMin; yMax=logMax; foundY=true;
                                }
                            } else if (usage == 0x38) {
                                // Only accept Wheel from the same report as X.
                                if (foundX && mouseReportId == reportId) {
                                    wBitOff=fieldOff; wBits=reportSize; hasWheel=true;
                                }
                            }
                        }
                    }
                }
                bitOff += (uint16_t)reportSize * reportCount;
            }
            // Clear local state after any Main item
            pendingUsageCnt = 0;
            pendingUsageMin = pendingUsageMax = 0;
            hasPendingRange = false;
        }
    }

    if (!foundX || !foundY) return false;
    out.valid          = true;
    out.reportId       = mouseReportId;
    out.buttonBitOffset = buttonBitOff;
    out.buttonCount    = (buttonCnt > 8) ? 8 : buttonCnt;
    out.xBitOffset     = xBitOff; out.xBitSize = xBits; out.xLogMin = xMin; out.xLogMax = xMax;
    out.yBitOffset     = yBitOff; out.yBitSize = yBits; out.yLogMin = yMin; out.yLogMax = yMax;
    out.wheelPresent   = hasWheel;
    out.wheelBitOffset = wBitOff; out.wheelBitSize = wBits;
    return true;
}

static const unsigned long DOUBLE_CLICK_WINDOW_MS = 800;

static const char *mouseButtonName(uint8_t buttonIndex) {
    switch (buttonIndex) {
        case 0: return "Left";
        case 1: return "Right";
        case 2: return "Middle";
        default: return "Unknown";
    }
}

void MyEspUsbHostClass::RecordMouseButtonEvent(const char *label, const char *action)
{
    snprintf(lastMouseButton.description,
             sizeof(lastMouseButton.description),
             "%s %s", label, action);
    lastMouseButton.time = millis();
}

void MyEspUsbHostClass::RecordMouseClick(uint8_t buttonIndex)
{
    unsigned long now = millis();
    unsigned long previousClickTime = lastMouseButton.lastClickTime;
    uint8_t previousButton = lastMouseButton.lastClickButton;

    lastMouseButton.totalClicks++;
    lastMouseButton.lastClickTime = now;
    lastMouseButton.lastClickButton = buttonIndex;
    lastMouseButton.lastClickIntervalMs = (previousClickTime != 0 && previousButton == buttonIndex)
        ? now - previousClickTime
        : 0;

    if (previousClickTime != 0 &&
        previousButton == buttonIndex &&
        now - previousClickTime <= DOUBLE_CLICK_WINDOW_MS) {
        RecordMouseDoubleClick(buttonIndex, now - previousClickTime);
    }
}

void MyEspUsbHostClass::RecordMouseDoubleClick(uint8_t buttonIndex, unsigned long intervalMs)
{
    unsigned long now = millis();
    lastMouseButton.lastDoubleClickTime = now;
    lastMouseButton.lastDoubleClickButton = buttonIndex;
    lastMouseButton.lastClickTime = now;
    lastMouseButton.lastClickButton = buttonIndex;
    lastMouseButton.lastClickIntervalMs = intervalMs;
    lastMouseButton.totalDoubleClicks++;
    snprintf(lastMouseButton.doubleClickDescription,
             sizeof(lastMouseButton.doubleClickDescription),
             "%s double-click", mouseButtonName(buttonIndex));
}

void MyEspUsbHostClass::LoadMouseSettings()
{
    JsonDocument doc = pmCommonLib.ConfigHandler.LoadConfigFile(MOUSE_SETTINGS_CONFIG_FILE);

    mouseSpeedPercent = (uint16_t)(doc["speedPercent"] | DEFAULT_MOUSE_SPEED_PERCENT);
    mouseClockHalfMicros = (uint16_t)(doc["clockHalfMicros"] | PS2Devices.GetMouseClockHalfPeriodMicros());
    mouseByteIntervalMicros = (uint16_t)(doc["byteIntervalMicros"] | PS2Devices.GetMouseByteIntervalMicros());

    if (mouseSpeedPercent < MIN_MOUSE_SPEED_PERCENT) mouseSpeedPercent = MIN_MOUSE_SPEED_PERCENT;
    if (mouseSpeedPercent > MAX_MOUSE_SPEED_PERCENT) mouseSpeedPercent = MAX_MOUSE_SPEED_PERCENT;
    if (mouseClockHalfMicros < 30 || mouseClockHalfMicros > 80) mouseClockHalfMicros = PS2Devices.GetMouseClockHalfPeriodMicros();
    if (mouseByteIntervalMicros < 50 || mouseByteIntervalMicros > 2000) mouseByteIntervalMicros = PS2Devices.GetMouseByteIntervalMicros();

    mouseScaledXCarry = 0;
    mouseScaledYCarry = 0;
    PS2Devices.SetMouseTiming(mouseClockHalfMicros, mouseByteIntervalMicros);
    pmLogging.LogLn("[USB] Mouse settings loaded: speed=" + String(mouseSpeedPercent) +
                    "% clockHalf=" + String(mouseClockHalfMicros) +
                    "us byteInterval=" + String(mouseByteIntervalMicros) + "us");
}

bool MyEspUsbHostClass::SaveMouseSettings()
{
    JsonDocument data;
    data["speedPercent"] = mouseSpeedPercent;
    data["clockHalfMicros"] = mouseClockHalfMicros;
    data["byteIntervalMicros"] = mouseByteIntervalMicros;

    if (pmCommonLib.ConfigHandler.SaveConfigFile(MOUSE_SETTINGS_CONFIG_FILE, data)) {
        pmLogging.LogLn("[USB] Mouse settings saved: speed=" + String(mouseSpeedPercent) +
                        "% clockHalf=" + String(mouseClockHalfMicros) +
                        "us byteInterval=" + String(mouseByteIntervalMicros) + "us");
        return true;
    } else {
        pmLogging.LogLn("[USB] Mouse settings save failed");
        return false;
    }
}

void MyEspUsbHostClass::SetMouseSpeedPercent(uint16_t speedPercent)
{
    if (speedPercent < MIN_MOUSE_SPEED_PERCENT) speedPercent = MIN_MOUSE_SPEED_PERCENT;
    if (speedPercent > MAX_MOUSE_SPEED_PERCENT) speedPercent = MAX_MOUSE_SPEED_PERCENT;
    mouseSpeedPercent = speedPercent;
    mouseScaledXCarry = 0;
    mouseScaledYCarry = 0;
}

uint16_t MyEspUsbHostClass::GetMouseSpeedPercent()
{
    uint16_t speedPercent = mouseSpeedPercent;
    if (speedPercent < MIN_MOUSE_SPEED_PERCENT) return MIN_MOUSE_SPEED_PERCENT;
    if (speedPercent > MAX_MOUSE_SPEED_PERCENT) return MAX_MOUSE_SPEED_PERCENT;
    return speedPercent;
}

void MyEspUsbHostClass::init()
{
    usb.setKeyboardLayout(ESP_USB_HOST_KEYBOARD_LAYOUT_DE_DE);

    usb.onDeviceConnected([](const EspUsbHostDeviceInfo& info) {
        MyEspUsbHost.cachedVid = info.vid;
        MyEspUsbHost.cachedPid = info.pid;
        MyEspUsbHost.ignoreMouseMoveUntil = millis() + 3000;
        strncpy(MyEspUsbHost.cachedManufacturer, info.manufacturer, sizeof(MyEspUsbHost.cachedManufacturer) - 1);
        MyEspUsbHost.cachedManufacturer[sizeof(MyEspUsbHost.cachedManufacturer) - 1] = '\0';
        strncpy(MyEspUsbHost.cachedProduct, info.product, sizeof(MyEspUsbHost.cachedProduct) - 1);
        MyEspUsbHost.cachedProduct[sizeof(MyEspUsbHost.cachedProduct) - 1] = '\0';
        pmLogging.LogLn("[USB] Connected: " + String(info.product) +
                        " VID=0x" + String(info.vid, HEX) +
                        " PID=0x" + String(info.pid, HEX));
    });

    usb.onDeviceDisconnected([](const EspUsbHostDeviceInfo& info) {
        pmLogging.LogLn("[USB] Disconnected");
        MyEspUsbHost.cachedVid = 0;
        MyEspUsbHost.cachedPid = 0;
        MyEspUsbHost.cachedManufacturer[0] = '\0';
        MyEspUsbHost.cachedProduct[0] = '\0';
        MyEspUsbHost.lastKey.time = 0;
        MyEspUsbHost.lastKey.keycode = 0;
        MyEspUsbHost.lastKey.totalPresses = 0;
        MyEspUsbHost.lastMouseMove.time = 0;
        MyEspUsbHost.lastMouseMove.x = 0;
        MyEspUsbHost.lastMouseMove.y = 0;
        MyEspUsbHost.lastMouseMove.wheel = 0;
        MyEspUsbHost.lastMouseMove.ps2X = 0;
        MyEspUsbHost.lastMouseMove.ps2Y = 0;
        MyEspUsbHost.lastMouseMove.totalX = 0;
        MyEspUsbHost.lastMouseMove.totalY = 0;
        MyEspUsbHost.lastMouseMove.totalPs2X = 0;
        MyEspUsbHost.lastMouseMove.totalPs2Y = 0;
        MyEspUsbHost.lastMouseMove.totalWheel = 0;
        MyEspUsbHost.lastMouseMove.totalMoves = 0;
        MyEspUsbHost.lastMouseButton.time = 0;
        MyEspUsbHost.lastMouseButton.description[0] = '\0';
        MyEspUsbHost.lastMouseButton.lastClickTime = 0;
        MyEspUsbHost.lastMouseButton.lastClickIntervalMs = 0;
        MyEspUsbHost.lastMouseButton.lastDoubleClickTime = 0;
        MyEspUsbHost.lastMouseButton.totalClicks = 0;
        MyEspUsbHost.lastMouseButton.totalDoubleClicks = 0;
        MyEspUsbHost.lastMouseButton.lastClickButton = 0xFF;
        MyEspUsbHost.lastMouseButton.lastDoubleClickButton = 0xFF;
        MyEspUsbHost.lastMouseButton.doubleClickDescription[0] = '\0';
        MyEspUsbHost.hidDesc.valid = false;
        MyEspUsbHost.hidDesc.length = 0;
        MyEspUsbHost.mouseLayout = {};
        MyEspUsbHost.ignoreMouseMoveUntil = 0;
    });

    usb.onHIDReportDescriptor([](const EspUsbHostHIDReportDescriptor& desc) {
        auto &d = MyEspUsbHost.hidDesc;
        d.interfaceNumber = desc.interfaceNumber;
        d.length = (desc.length <= sizeof(d.data)) ? desc.length : (uint16_t)sizeof(d.data);
        memcpy(d.data, desc.data, d.length);
        d.valid = true;

        auto &ml = MyEspUsbHost.mouseLayout;
        ml = {};  // reset before re-parsing
        bool ok = parseHIDMouseLayout(d.data, d.length, ml);
        if (ok) {
            pmLogging.LogLn("[USB] Mouse layout: reportId=" + String(ml.reportId) +
                            " buttons=" + String(ml.buttonCount) + "@bit" + String(ml.buttonBitOffset) +
                            " X=" + String(ml.xBitSize) + "bit@" + String(ml.xBitOffset) +
                            " Y=" + String(ml.yBitSize) + "bit@" + String(ml.yBitOffset) +
                            (ml.wheelPresent ? " Wheel=" + String(ml.wheelBitSize) + "bit@" + String(ml.wheelBitOffset) : String(" noWheel")));
        } else {
            pmLogging.LogLn("[USB] HID descriptor: iface=" + String(desc.interfaceNumber) +
                            " len=" + String(desc.length) + " (no mouse layout found, using library defaults)");
        }
    });

    usb.onHIDInput([](const EspUsbHostHIDInput& input) {
        // Capture the interface subclass/protocol alongside the descriptor
        if (MyEspUsbHost.hidDesc.interfaceNumber == input.interfaceNumber) {
            MyEspUsbHost.hidDesc.subClass = input.subclass;
            MyEspUsbHost.hidDesc.protocol = input.protocol;
        }

        if (looksLikeReportIdMousePacket((const uint8_t *)input.data,
                                         (uint16_t)input.length,
                                         input.data ? input.data[0] : 0)) {
            static uint8_t prevReportIdButtons = 0;
            uint8_t buttons = 0;
            int16_t x = 0;
            int16_t y = 0;
            int8_t wheel = 0;
            if (!decodeReportIdMousePacket((const uint8_t *)input.data,
                                           (uint16_t)input.length,
                                           x,
                                           y,
                                           wheel,
                                           buttons)) {
                return;
            }

            storeMouseRawReport((const uint8_t *)input.data,
                                input.length,
                                (const uint8_t *)input.data,
                                input.length,
                                x,
                                y,
                                wheel,
                                input.data[0],
                                buttons);

            handleDecodedMouseMove(x, y, wheel);
            handleDecodedMouseButtons(prevReportIdButtons, buttons);
            prevReportIdButtons = buttons;
        }
    });

    usb.onKeyboard([](const EspUsbHostKeyboardEvent& evt) {
        static uint8_t lastModifiers = 0;

        // Track modifier key state changes and forward to PS/2
        uint8_t mod = evt.modifiers;
        if (mod != lastModifiers) {
            uint8_t changed = mod ^ lastModifiers;
            auto chk = [&](uint8_t bit, esp32_ps2dev::scancodes::Key key) {
                if (!(changed & bit)) return;
                if (mod & bit) PS2Devices.KeyDown(key);
                else           PS2Devices.KeyUp(key);
            };
            chk(0x01, esp32_ps2dev::scancodes::Key::K_LCTRL);
            chk(0x02, esp32_ps2dev::scancodes::Key::K_LSHIFT);
            chk(0x04, esp32_ps2dev::scancodes::Key::K_LALT);
            chk(0x10, esp32_ps2dev::scancodes::Key::K_RCTRL);
            chk(0x20, esp32_ps2dev::scancodes::Key::K_RSHIFT);
            chk(0x40, esp32_ps2dev::scancodes::Key::K_RALT);
            lastModifiers = mod;
        }

        // Regular key press/release (skip modifier keycodes 0xE0+ to avoid double-fire)
        if (evt.keycode == 0 || evt.keycode >= 0xE0) return;

        if (evt.pressed) {
            pmLogging.LogLn("Key down: " + String(evt.keycode));
            PS2Devices.KeyDown(USBKeyCodeToPS2ScanCode(evt.keycode));
            MyEspUsbHost.lastKey.time = millis();
            MyEspUsbHost.lastKey.keycode = evt.keycode;
            MyEspUsbHost.lastKey.totalPresses++;
        }
        if (evt.released) {
            PS2Devices.KeyUp(USBKeyCodeToPS2ScanCode(evt.keycode));
        }
    });

    usb.onMouse([](const EspUsbHostMouseEvent& evt) {
        // Extract mouse axes and buttons using the layout parsed from the HID report descriptor.
        // Falls back to the library-provided values if no layout was parsed.
        static uint8_t prevButtons = 0;

        int16_t x     = evt.x;
        int16_t y     = evt.y;
        int8_t  wheel = (int8_t)constrain(evt.wheel, -128, 127);
        uint8_t buttons = evt.buttons & 0x07;

        uint8_t reportIdPacketButtons = 0;
        bool hasReportIdPacketButtons = extractReportIdPacketButtons(evt, reportIdPacketButtons);
        bool reportHandledByHidInput = hasReportIdPacketButtons;
        if (hasReportIdPacketButtons) {
            buttons = reportIdPacketButtons;
        }

        const auto &layout = MyEspUsbHost.mouseLayout;
        if (layout.valid) {
            const uint8_t *rpt = nullptr;
            uint16_t rptLen = 0;
            if (selectMouseReportPayload(evt, layout, rpt, rptLen)) {
                x = (int16_t)extractBitsSigned(rpt, rptLen, layout.xBitOffset, layout.xBitSize);
                y = (int16_t)extractBitsSigned(rpt, rptLen, layout.yBitOffset, layout.yBitSize);
                if (layout.wheelPresent)
                    wheel = (int8_t)constrain(extractBitsSigned(rpt, rptLen, layout.wheelBitOffset, layout.wheelBitSize), -128, 127);
                else
                    wheel = 0;
                if (!hasReportIdPacketButtons) {
                    buttons = 0;
                    for (uint8_t b = 0; b < layout.buttonCount && b < 8; b++) {
                        if (extractBitsSigned(rpt, rptLen, layout.buttonBitOffset + b, 1) & 1)
                            buttons |= (uint8_t)(1u << b);
                    }
                }
            }
        } else if (hasReportIdPacketButtons) {
            buttons = reportIdPacketButtons;
        }
        if (hasReportIdPacketButtons) {
            buttons = reportIdPacketButtons;
        }

        // Mask to the 3 buttons PS/2 actually models before change detection.
        // Extra buttons (bits 3+) from high-button mice must not create spurious
        // press/release events or corrupt prevButtons state.
        buttons &= 0x07;

        if (!reportHandledByHidInput) {
            storeMouseRawReport((const uint8_t *)evt.rawData,
                                evt.rawLength,
                                (const uint8_t *)evt.reportData,
                                evt.reportLength,
                                evt.x,
                                evt.y,
                                (int8_t)constrain(evt.wheel, -128, 127),
                                evt.buttons,
                                buttons);
        }

        uint8_t previousButtons = prevButtons;
        prevButtons = buttons;

        bool moved       = (x != 0 || y != 0 || wheel != 0);
        bool btnsChanged = (buttons != previousButtons);

        if (moved && !reportHandledByHidInput) {
            handleDecodedMouseMove(x, y, wheel);
        }

        if (btnsChanged && !reportHandledByHidInput) {
            handleDecodedMouseButtons(previousButtons, buttons);
        }
    });

    // Start the USB host — it manages its own internal task on Core 1
    EspUsbHostConfig config;
    config.taskCore = 1;
    usb.begin(config);
}

void MyEspUsbHostClass::DisplayInfo()
{
    pmLogging.LogLn("[*] USB information:");
    EspUsbHostDeviceInfo devices[ESP_USB_HOST_MAX_DEVICES];
    size_t devCount = usb.getDevices(devices, ESP_USB_HOST_MAX_DEVICES);
    if (devCount == 0) {
        pmLogging.LogLn("  No devices connected");
        return;
    }
    for (size_t d = 0; d < devCount; d++) {
        pmLogging.LogLn("  Device: VID=0x" + String(devices[d].vid, HEX) +
                        " PID=0x" + String(devices[d].pid, HEX) +
                        " " + String(devices[d].product));
        EspUsbHostInterfaceInfo intfs[ESP_USB_HOST_MAX_INTERFACES];
        size_t n = usb.getInterfaces(devices[d].address, intfs, ESP_USB_HOST_MAX_INTERFACES);
        for (size_t i = 0; i < n; i++) {
            pmLogging.LogLn("    IF " + String(intfs[i].number) +
                           " Class=0x" + String(intfs[i].interfaceClass, HEX) +
                           " Sub=0x" + String(intfs[i].interfaceSubClass, HEX) +
                           " Proto=0x" + String(intfs[i].interfaceProtocol, HEX));
        }
    }
}

MyEspUsbHostClass MyEspUsbHost;
