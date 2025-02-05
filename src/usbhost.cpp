#include "usbhost.h"    

void MyEspUsbHostClass::onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier) {
    

    String str = "Keyboard: " +  String(ascii) + " " + String(keycode) + " " + String(modifier);
    WebSerialLogger.println(str);
    

    switch (keycode)
    {
      case 69:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F12);
        break;
      case 68:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F11);
        break;
      case 67:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F10);
        break;
      case 66:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F9);
        break;
      case 65:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F8);
        break;
      case 64:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F7);
        break;
      case 63:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F6);
        break;
      case 62:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F5);
        break;
      case 61:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F4);
        break;
      case 60:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F3);
        break;
      case 59:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F2);
        break;
      case 58:
        PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F1);
        break;
      
      default:
        PS2Devices.Type((char)ascii);
        break;
    }
  };

void MyEspUsbHostClass::onMouseMove(hid_mouse_report_t report) {

    String str = "onMouseMove: " + String(report.x) + " " + String(report.y) + " " + String(report.wheel);

    Serial.println(str);
    PS2Devices.MoveMouse((int16_t)report.x, (int16_t)-report.y, report.wheel);
};

void MyEspUsbHostClass::onMouseButtons(hid_mouse_report_t report, uint8_t last_buttons)
{
    // LEFT
    if (!(last_buttons & MOUSE_BUTTON_LEFT) && (report.buttons & MOUSE_BUTTON_LEFT)) {
      Serial.println("Mouse LEFT Click");
      PS2Devices.PressMouseButton(esp32_ps2dev::PS2Mouse::Button::LEFT);
    }
    if ((last_buttons & MOUSE_BUTTON_LEFT) && !(report.buttons & MOUSE_BUTTON_LEFT)) {
      Serial.println("Mouse LEFT Release");
      PS2Devices.ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button::LEFT);
    }

    // RIGHT
    if (!(last_buttons & MOUSE_BUTTON_RIGHT) && (report.buttons & MOUSE_BUTTON_RIGHT)) {
      Serial.println("Mouse RIGHT Click");
      PS2Devices.PressMouseButton(esp32_ps2dev::PS2Mouse::Button::LEFT);
    }
    if ((last_buttons & MOUSE_BUTTON_RIGHT) && !(report.buttons & MOUSE_BUTTON_RIGHT)) {
      Serial.println("Mouse RIGHT Release");
      PS2Devices.ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button::LEFT);
    }

    // MIDDLE
    if (!(last_buttons & MOUSE_BUTTON_MIDDLE) && (report.buttons & MOUSE_BUTTON_MIDDLE)) {
      Serial.println("Mouse MIDDLE Click");
      PS2Devices.PressMouseButton(esp32_ps2dev::PS2Mouse::Button::LEFT);
    }
    if ((last_buttons & MOUSE_BUTTON_MIDDLE) && !(report.buttons & MOUSE_BUTTON_MIDDLE)) {
      Serial.println("Mouse MIDDLE Release");
      PS2Devices.ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button::LEFT);
    }

}

void MyEspUsbHostClass::onData(const usb_transfer_t *transfer)
{
    Serial.printf("USB Data received: %d\n bytes", transfer->data_buffer_size);

    //WebSerialLogger.println("onData");
}

void MyEspUsbHostClass::onGone(const usb_host_client_event_msg_t *eventMsg)
{

    Serial.println("onGone");
}

void MyEspUsbHostClass::init()
{
    this->begin();
    setHIDLocal(HID_LOCAL);
}

void MyEspUsbHostClass::DisplayInfo()
{
    WebSerialLogger.print("[*] USB information:");
    
    for(int i = 0; i < 5; i++)
    {
      endpoint_data_t ep = this->endpoint_data_list[i];

      String str = String(ep.bInterfaceNumber) + " " + String(ep.bInterfaceClass) + " " + String(ep.bInterfaceProtocol) + " " + String(ep.bCountryCode);
      WebSerialLogger.println(str);
    }
}

MyEspUsbHostClass MyEspUsbHost;