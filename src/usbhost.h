#include "Arduino.h"
#include "webserial.hpp"
#include "ps2devices.h"


#ifndef USBHOST_H
#define USBHOST_H

#include "EspUsbHost.h"

#define HID_LOCAL HID_LOCAL_German

class MyEspUsbHostClass : public EspUsbHost {
  
    private:
        void onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier);

        void onMouse(hid_mouse_report_t report, uint8_t last_buttons);
        void onMouseMove(hid_mouse_report_t report);
        void onMouseButtons(hid_mouse_report_t report, uint8_t last_buttons);
        void onConfigured();
        void onConnect();
    public:
        void DisplayInfo();
        void init();
};

extern MyEspUsbHostClass MyEspUsbHost;
#endif