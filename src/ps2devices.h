#include <Arduino.h>
#include <esp32-ps2dev.h>

#ifndef PS2DEVICES_H
#define PS2DEVICES_H

class PS2DevicesClass
{
    private:
        esp32_ps2dev::PS2Mouse *_mouse;
        esp32_ps2dev::PS2Keyboard *_keyboard;
       
    public:
        void InitKeyboard(int clock, int data);
        void InitMouse(int clock, int data);

        void Type(esp32_ps2dev::scancodes::Key key);
        void Type(const char *text);
        void MoveMouse(int x, int y, int wheel);
        void PressMouse(esp32_ps2dev::PS2Mouse::Button button);
};


extern PS2DevicesClass PS2Devices;
#endif