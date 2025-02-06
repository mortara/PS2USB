#include "usbhost.h"    

esp32_ps2dev::scancodes::Key MyEspUsbHostClass::USBKeyCodeToPS2ScanCode(uint8_t keycode)
{
    switch (keycode)
    {
      case 4:
        return esp32_ps2dev::scancodes::Key::K_A;
        break;
      case 5:
        return esp32_ps2dev::scancodes::Key::K_B;
        break;
      case 6:
        return esp32_ps2dev::scancodes::Key::K_C;
        break;
      case 7:
        return esp32_ps2dev::scancodes::Key::K_D;
        break;
      case 8:
        return esp32_ps2dev::scancodes::Key::K_E;
        break;
      case 9:
        return esp32_ps2dev::scancodes::Key::K_F;
        break;
      case 10:
        return esp32_ps2dev::scancodes::Key::K_G;
        break;
      case 11:  
        return esp32_ps2dev::scancodes::Key::K_H;
        break;
      case 12:  
        return esp32_ps2dev::scancodes::Key::K_I;
        break;
      case 13:
        return esp32_ps2dev::scancodes::Key::K_J;
        break;
      case 14:
        return esp32_ps2dev::scancodes::Key::K_K;
        break;
      case 15:
        return esp32_ps2dev::scancodes::Key::K_L;
        break;
      case 16:
        return esp32_ps2dev::scancodes::Key::K_M;
        break;
      case 17:
        return esp32_ps2dev::scancodes::Key::K_N;
        break;
      case 18:
        return esp32_ps2dev::scancodes::Key::K_O;
        break;
      case 19:
        return esp32_ps2dev::scancodes::Key::K_P;
        break;
      case 20:
        return esp32_ps2dev::scancodes::Key::K_Q;
        break;
      case 21:
        return esp32_ps2dev::scancodes::Key::K_R;
        break;
      case 22:
        return esp32_ps2dev::scancodes::Key::K_S;
        break;
      case 23:
        return esp32_ps2dev::scancodes::Key::K_T;
        break;
      case 24:
        return esp32_ps2dev::scancodes::Key::K_U;
        break;
      case 25:
        return esp32_ps2dev::scancodes::Key::K_V;
        break;
      case 26:
        return esp32_ps2dev::scancodes::Key::K_W;
        break;
      case 27:
        return esp32_ps2dev::scancodes::Key::K_X;
        break;
      case 28:
        return esp32_ps2dev::scancodes::Key::K_Y;
        break;
      case 29:
        return esp32_ps2dev::scancodes::Key::K_Z;
        break;
      case 30:
        return esp32_ps2dev::scancodes::Key::K_1;
        break;
      case 31:
        return esp32_ps2dev::scancodes::Key::K_2;
        break;
      case 32:
        return esp32_ps2dev::scancodes::Key::K_3;
        break;
      case 33:
        return esp32_ps2dev::scancodes::Key::K_4;
        break;
      case 34:
        return esp32_ps2dev::scancodes::Key::K_5;
        break;
      case 35:
        return esp32_ps2dev::scancodes::Key::K_6;
        break;
      case 36:
        return esp32_ps2dev::scancodes::Key::K_7;
        break;
      case 37:
        return esp32_ps2dev::scancodes::Key::K_8;
        break;
      case 38:
        return esp32_ps2dev::scancodes::Key::K_9;
        break;
      case 39:
        return esp32_ps2dev::scancodes::Key::K_0;
        break;
      case 40:
        return esp32_ps2dev::scancodes::Key::K_RETURN;
        break;
      case 41:
        return esp32_ps2dev::scancodes::Key::K_ESCAPE;
        break;
      case 42:
        return esp32_ps2dev::scancodes::Key::K_BACKSPACE;
        break;
      case 43:
        return esp32_ps2dev::scancodes::Key::K_TAB;
        break;
      case 44:
        return esp32_ps2dev::scancodes::Key::K_SPACE;
        break;
      case 45:
        return esp32_ps2dev::scancodes::Key::K_MINUS;
        break;
      case 46:
        return esp32_ps2dev::scancodes::Key::K_EQUALS;
        break;
      case 47:
        return esp32_ps2dev::scancodes::Key::K_LEFTBRACKET;
        break;
      case 48:
        return esp32_ps2dev::scancodes::Key::K_RIGHTBRACKET;
        break;                      

      case 58:
        return esp32_ps2dev::scancodes::Key::K_F1;
        break;
      case 59:
        return esp32_ps2dev::scancodes::Key::K_F2;
        break;
      case 60:
        return esp32_ps2dev::scancodes::Key::K_F3;
        break;
      case 61:
        return esp32_ps2dev::scancodes::Key::K_F4;
        break;
      case 62:
        return esp32_ps2dev::scancodes::Key::K_F5;
        break;
      case 63:
        return esp32_ps2dev::scancodes::Key::K_F6;
        break;
      case 64:
        return esp32_ps2dev::scancodes::Key::K_F7;
        break;
      case 65:
        return esp32_ps2dev::scancodes::Key::K_F8;
        break;
      case 66:
        return esp32_ps2dev::scancodes::Key::K_F9;
        break;
      case 67:
        return esp32_ps2dev::scancodes::Key::K_F10;
        break;
      case 68:
        return esp32_ps2dev::scancodes::Key::K_F11;
        break;
      case 69:
        return esp32_ps2dev::scancodes::Key::K_F12;
        break;
      
      default:
        return (esp32_ps2dev::scancodes::Key)keycode;
        break;
    }
}

void MyEspUsbHostClass::onKeyboard(hid_keyboard_report_t report, hid_keyboard_report_t last_report)
{
    //String str = "Keyboard: " +  String(report.keycode[0]) + " " + String(report.modifier);
    //WebSerialLogger.println(str);

    if(report.modifier != last_report.modifier)
    {
      // Shift
      if((report.modifier & KEYBOARD_MODIFIER_LEFTSHIFT) || (report.modifier & KEYBOARD_MODIFIER_RIGHTSHIFT))
      {
        PS2Devices.KeyDown(esp32_ps2dev::scancodes::Key::K_LSHIFT);
      }
      else if((last_report.modifier & KEYBOARD_MODIFIER_LEFTSHIFT) || (last_report.modifier & KEYBOARD_MODIFIER_RIGHTSHIFT))
      {
        PS2Devices.KeyUp(esp32_ps2dev::scancodes::Key::K_LSHIFT);
      }

      // Ctrl
      if((report.modifier & KEYBOARD_MODIFIER_LEFTCTRL) || (report.modifier & KEYBOARD_MODIFIER_RIGHTCTRL))
      {
        PS2Devices.KeyDown(esp32_ps2dev::scancodes::Key::K_LCTRL);
      }
      else if((last_report.modifier & KEYBOARD_MODIFIER_LEFTCTRL) || (last_report.modifier & KEYBOARD_MODIFIER_RIGHTCTRL))
      {
        PS2Devices.KeyUp(esp32_ps2dev::scancodes::Key::K_LCTRL);
      }

      // Alt
      if((report.modifier & KEYBOARD_MODIFIER_LEFTALT) || (report.modifier & KEYBOARD_MODIFIER_RIGHTALT))
      {
        PS2Devices.KeyDown(esp32_ps2dev::scancodes::Key::K_LALT);
      }
      else if((last_report.modifier & KEYBOARD_MODIFIER_LEFTALT) || (last_report.modifier & KEYBOARD_MODIFIER_RIGHTALT))
      {
        PS2Devices.KeyUp(esp32_ps2dev::scancodes::Key::K_LALT);
      }

      // RIGHT SHIFT
      if((report.modifier & KEYBOARD_MODIFIER_RIGHTSHIFT) || (report.modifier & KEYBOARD_MODIFIER_RIGHTSHIFT))
      {
        PS2Devices.KeyDown(esp32_ps2dev::scancodes::Key::K_RSHIFT);
      }
      else if((last_report.modifier & KEYBOARD_MODIFIER_RIGHTSHIFT) || (last_report.modifier & KEYBOARD_MODIFIER_RIGHTSHIFT))
      {
        PS2Devices.KeyUp(esp32_ps2dev::scancodes::Key::K_RSHIFT);
      }
    }

    for (int i = 0; i < 6; i++) {
      if (report.keycode[i] != 0 && last_report.keycode[i] == 0) {
        // Type
        esp32_ps2dev::scancodes::Key scancode = USBKeyCodeToPS2ScanCode(report.keycode[i]);
        PS2Devices.KeyDown(scancode);
      }
      else if (report.keycode[i] == 0 && last_report.keycode[i] != 0) {
        // Release
        esp32_ps2dev::scancodes::Key scancode = USBKeyCodeToPS2ScanCode(last_report.keycode[i]);
        PS2Devices.KeyUp(scancode);
      }
    }
} 

void MyEspUsbHostClass::onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier) {
    
 
    
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