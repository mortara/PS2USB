#include "usbhost.h"
#include "ps2devices.h"
#include "pmCommonLib.hpp"

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

void MyEspUsbHostClass::init()
{
    usb.setKeyboardLayout(ESP_USB_HOST_KEYBOARD_LAYOUT_DE_DE);

    usb.onDeviceConnected([](const EspUsbHostDeviceInfo& info) {
        MyEspUsbHost.cachedVid = info.vid;
        MyEspUsbHost.cachedPid = info.pid;
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
        MyEspUsbHost.lastMouseMove.totalMoves = 0;
        MyEspUsbHost.lastMouseButton.time = 0;
        MyEspUsbHost.lastMouseButton.description[0] = '\0';
        MyEspUsbHost.hidDesc.valid = false;
        MyEspUsbHost.hidDesc.length = 0;
    });

    usb.onHIDReportDescriptor([](const EspUsbHostHIDReportDescriptor& desc) {
        auto &d = MyEspUsbHost.hidDesc;
        d.interfaceNumber = desc.interfaceNumber;
        d.length = (desc.length <= sizeof(d.data)) ? desc.length : (uint16_t)sizeof(d.data);
        memcpy(d.data, desc.data, d.length);
        d.valid = true;
        pmLogging.LogLn("[USB] HID descriptor: iface=" + String(desc.interfaceNumber) +
                        " len=" + String(desc.length));
    });

    usb.onHIDInput([](const EspUsbHostHIDInput& input) {
        // Capture the interface subclass/protocol alongside the descriptor
        if (MyEspUsbHost.hidDesc.interfaceNumber == input.interfaceNumber) {
            MyEspUsbHost.hidDesc.subClass = input.subclass;
            MyEspUsbHost.hidDesc.protocol = input.protocol;
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
        // Store in ring buffer for the web diagnostic view.
        uint8_t idx = MyEspUsbHost.mouseRawHead;
        auto &entry = MyEspUsbHost.mouseRawLog[idx];
        entry.time       = millis();
        entry.rawLen     = (uint8_t)min(evt.rawLength,  (size_t)MyEspUsbHostClass::MOUSE_RAW_MAX_BYTES);
        entry.rptLen     = (uint8_t)min(evt.reportLength,(size_t)MyEspUsbHostClass::MOUSE_RAW_MAX_BYTES);
        if (evt.rawData)    memcpy(entry.rawData, evt.rawData,    entry.rawLen);
        if (evt.reportData) memcpy(entry.rptData, evt.reportData, entry.rptLen);
        entry.libX       = evt.x;
        entry.libY       = evt.y;
        entry.libWheel   = (int8_t)constrain(evt.wheel, -128, 127);
        entry.libButtons = evt.buttons;
        MyEspUsbHost.mouseRawHead = (idx + 1) % MyEspUsbHostClass::MOUSE_RAW_LOG_SIZE;

        // This mouse uses a 12-bit packed report (after the library strips the 0x02 report ID):
        //   byte[0] = buttons
        //   byte[1] = X[7:0]
        //   byte[2] = Y[7:0]
        //   byte[3] = Y[11:8] (low nibble) | X[11:8] (high nibble)
        //   byte[4] = scroll wheel (8-bit)
        //
        // The library misreads byte[3] as the wheel field, which is actually the
        // packed high nibbles of X and Y.  This would send wheel=15 to the PS/2 host
        // on every Y=-1 movement.  Parse reportData directly instead.
        uint8_t buttons         = evt.buttons & 0x07;
        uint8_t previousButtons = evt.previousButtons & 0x07;

        int16_t x = evt.x;  // int8(rpt[1]) is correct for |X| <= 127
        int16_t y = evt.y;  // int8(rpt[2]) is correct for |Y| <= 127
        int8_t  wheel = 0;

        // Layout after report ID strip (confirmed from raw capture):
        //   r[0] = buttons
        //   r[1] = scroll wheel (8-bit; 0 when not scrolling)
        //   r[2] = X[7:0]
        //   r[3] = X[11:8] (low nibble) | Y[3:0] (high nibble)
        //   r[4] = Y[11:4]
        if (evt.reportData && evt.reportLength >= 5) {
            const uint8_t *r = (const uint8_t *)evt.reportData;
            uint16_t xRaw = (uint16_t)r[2] | ((uint16_t)(r[3] & 0x0F) << 8);
            uint16_t yRaw = (uint16_t)(r[3] >> 4) | ((uint16_t)r[4] << 4);
            x = (xRaw & 0x800) ? (int16_t)(xRaw | 0xF000) : (int16_t)xRaw;
            y = (yRaw & 0x800) ? (int16_t)(yRaw | 0xF000) : (int16_t)yRaw;
            wheel = (int8_t)r[1];
        }

        bool moved       = (x != 0 || y != 0 || wheel != 0);
        bool btnsChanged = (buttons != previousButtons);

        if (moved) {
            // Clamp to ±127 before handing to the PS/2 library.
            // The PS/2 protocol sets overflow bits when |value| > 255, and many
            // retro PS/2 host drivers react to overflow with wild cursor jumps or
            // a mouse reset. ±127 also matches what the old boot-protocol int8_t
            // gave us, and survives the PS/2 host's optional 2:1 scale mode
            // (127 × 2 = 254, safely below the 255 overflow threshold).
            auto clamp7 = [](int16_t v) -> int16_t {
                return v > 127 ? 127 : (v < -127 ? -127 : v);
            };
            PS2Devices.MoveMouse(clamp7(x), clamp7((int16_t)-y), wheel);
            MyEspUsbHost.lastMouseMove.time = millis();
            MyEspUsbHost.lastMouseMove.x = x;
            MyEspUsbHost.lastMouseMove.y = y;
            MyEspUsbHost.lastMouseMove.wheel = wheel;
            MyEspUsbHost.lastMouseMove.totalMoves++;
        }

        if (btnsChanged) {
            uint8_t changed = buttons ^ previousButtons;

            if (changed & 0x01) {
                if (buttons & 0x01) {
                    pmLogging.LogLn("Mouse LEFT pressed");
                    PS2Devices.PressMouseButton(esp32_ps2dev::PS2Mouse::Button::LEFT);
                    strncpy(MyEspUsbHost.lastMouseButton.description, "Left pressed", 31);
                } else {
                    pmLogging.LogLn("Mouse LEFT released");
                    PS2Devices.ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button::LEFT);
                    strncpy(MyEspUsbHost.lastMouseButton.description, "Left released", 31);
                }
            }
            if (changed & 0x02) {
                if (buttons & 0x02) {
                    pmLogging.LogLn("Mouse RIGHT pressed");
                    PS2Devices.PressMouseButton(esp32_ps2dev::PS2Mouse::Button::RIGHT);
                    strncpy(MyEspUsbHost.lastMouseButton.description, "Right pressed", 31);
                } else {
                    pmLogging.LogLn("Mouse RIGHT released");
                    PS2Devices.ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button::RIGHT);
                    strncpy(MyEspUsbHost.lastMouseButton.description, "Right released", 31);
                }
            }
            if (changed & 0x04) {
                if (buttons & 0x04) {
                    pmLogging.LogLn("Mouse MIDDLE pressed");
                    PS2Devices.PressMouseButton(esp32_ps2dev::PS2Mouse::Button::MIDDLE);
                    strncpy(MyEspUsbHost.lastMouseButton.description, "Middle pressed", 31);
                } else {
                    pmLogging.LogLn("Mouse MIDDLE released");
                    PS2Devices.ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button::MIDDLE);
                    strncpy(MyEspUsbHost.lastMouseButton.description, "Middle released", 31);
                }
            }
            MyEspUsbHost.lastMouseButton.time = millis();
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
