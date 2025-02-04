#include "Arduino.h"
#include "webserial.hpp"
#include "ps2devices.h"
#include "EspUsbHost.h"

#ifndef USBHOST_H
#define USBHOST_H

#define HID_LOCAL HID_LOCAL_German

class MyEspUsbHostClass : public EspUsbHost {
  
    private:
        void onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier);
        void onMouseMoved(int16_t x, int16_t y, int8_t wheel);
        void onConfigured();
        void onConnect();
    public:
        void begin();
};

extern MyEspUsbHostClass MyEspUsbHost;
#endif