#include "usbhost.h"
#include "ps2devices.h"
#include "pmCommonLib.hpp"

static esp32_ps2dev::scancodes::Key USBKeyCodeToPS2ScanCode(uint8_t keycode)
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
      // Modifier keycodes 0xE0–0xE7
      case 0xE0: return esp32_ps2dev::scancodes::Key::K_LCTRL;
      case 0xE1: return esp32_ps2dev::scancodes::Key::K_LSHIFT;
      case 0xE2: return esp32_ps2dev::scancodes::Key::K_LALT;
      case 0xE4: return esp32_ps2dev::scancodes::Key::K_LCTRL;  // RCTRL
      case 0xE5: return esp32_ps2dev::scancodes::Key::K_RSHIFT;
      case 0xE6: return esp32_ps2dev::scancodes::Key::K_LALT;   // RALT
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
        // reset raw-format log flag so reconnect logs the format again
        // (done via onMouse static — no direct access; harmless, will not re-log until reboot)
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
            chk(0x10, esp32_ps2dev::scancodes::Key::K_LCTRL);  // RCTRL
            chk(0x20, esp32_ps2dev::scancodes::Key::K_RSHIFT);
            chk(0x40, esp32_ps2dev::scancodes::Key::K_LALT);   // RALT
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
        static uint8_t prevButtons = 0;
        static bool loggedFormat = false;

        // Log both raw and report data on first event so the format can be verified
        if (!loggedFormat) {
            String msg = "[Mouse] reportData(len=" + String(evt.reportLength) + "):";
            if (evt.reportData) {
                for (size_t i = 0; i < min((size_t)6, evt.reportLength); i++)
                    msg += " " + String(evt.reportData[i], HEX);
            } else {
                msg += " NULL";
            }
            msg += " | rawData(len=" + String(evt.rawLength) + "):";
            if (evt.rawData) {
                for (size_t i = 0; i < min((size_t)6, evt.rawLength); i++)
                    msg += " " + String(evt.rawData[i], HEX);
            } else {
                msg += " NULL";
            }
            pmLogging.LogLn(msg);
            loggedFormat = true;
        }

        // Prefer reportData (library already stripped the Report ID).
        // Fall back to rawData; skip the first byte (Report ID) only when the payload is
        // long enough that the ID prefix is highly likely (>= 6 bytes suggests ID + 16-bit data).
        const uint8_t *data = nullptr;
        size_t         len  = 0;
        if (evt.reportData && evt.reportLength >= 3) {
            data = evt.reportData;
            len  = evt.reportLength;
        } else if (evt.rawData && evt.rawLength >= 6) {
            data = evt.rawData + 1;   // skip Report ID at byte 0
            len  = evt.rawLength - 1;
        } else if (evt.rawData && evt.rawLength >= 3) {
            data = evt.rawData;
            len  = evt.rawLength;
        } else {
            return;
        }

        // Axis layout detection from payload length:
        //
        //  len >= 6: [extra, buttons, X_lo, X_hi, Y_lo, Y_hi, wheel?]
        //            The leading byte is an un-stripped report ID or a padding/extension
        //            byte. Buttons start at byte 1.
        //
        //  len == 5: [buttons, X_lo, X_hi, Y_lo, Y_hi]
        //            Standard 16-bit, no extra byte.
        //
        //  len 3–4:  [buttons, X8, Y8, wheel?]
        //            8-bit per axis (older / boot-protocol mice).
        uint8_t buttons;
        int16_t x, y;
        int8_t  wheel;
        if (len >= 6) {
            buttons = data[1] & 0x07;
            x       = (int16_t)((uint16_t)data[2] | ((uint16_t)data[3] << 8));
            y       = (int16_t)((uint16_t)data[4] | ((uint16_t)data[5] << 8));
            wheel   = (len >= 7) ? (int8_t)data[6] : 0;
        } else if (len == 5) {
            buttons = data[0] & 0x07;
            x       = (int16_t)((uint16_t)data[1] | ((uint16_t)data[2] << 8));
            y       = (int16_t)((uint16_t)data[3] | ((uint16_t)data[4] << 8));
            wheel   = 0;
        } else {
            buttons = data[0] & 0x07;
            x       = (int8_t)data[1];
            y       = (int8_t)data[2];
            wheel   = (len >= 4) ? (int8_t)data[3] : 0;
        }

        bool moved       = (x != 0 || y != 0 || wheel != 0);
        bool btnsChanged = (buttons != prevButtons);

        if (moved) {
            PS2Devices.MoveMouse(x, (int16_t)-y, wheel);
            MyEspUsbHost.lastMouseMove.time = millis();
            MyEspUsbHost.lastMouseMove.x = x;
            MyEspUsbHost.lastMouseMove.y = y;
            MyEspUsbHost.lastMouseMove.wheel = wheel;
            MyEspUsbHost.lastMouseMove.totalMoves++;
        }

        if (btnsChanged) {
            uint8_t changed = buttons ^ prevButtons;

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
            prevButtons = buttons;
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
