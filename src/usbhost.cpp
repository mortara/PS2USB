#include "usbhost.h"    

void MyEspUsbHostClass::onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier) {
    if (' ' <= ascii && ascii <= '~') {
      WebSerialLogger.printf("%c", ascii);
    } else if (ascii == '\r') {
      WebSerialLogger.println();
    }
    switch (keycode)
    {
      case '0':
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_0);
        break;
      
      default:
        PS2Devices.Type((esp32_ps2dev::scancodes::Key)keycode);
        break;
    }
  };

void MyEspUsbHostClass::onMouseMoved(int16_t x, int16_t y, int8_t wheel) {
    WebSerialLogger.printf("Mouse moved: x=%d, y=%d, wheel=%d\n", x, y, wheel);
    PS2Devices.MoveMouse(x, y, wheel);
};

void MyEspUsbHostClass::onConfigured() {
    WebSerialLogger.println("USB device configured");
};

void MyEspUsbHostClass::onConnect() {
      WebSerialLogger.println("USB device connected");
};

void MyEspUsbHostClass::begin() {
    EspUsbHost::begin();
    setHIDLocal(HID_LOCAL);
}

MyEspUsbHostClass MyEspUsbHost;