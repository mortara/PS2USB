#include <Arduino.h>
#include <PS2Mouse.hpp>
#include <PS2Keyboard.hpp>

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

        void KeyUp(esp32_ps2dev::scancodes::Key key);
        void KeyDown(esp32_ps2dev::scancodes::Key key);

        void Type(esp32_ps2dev::scancodes::Key key);
        void Type(const char *text);
        void Type(char ch);
        void MoveMouse(int16_t x, int16_t y, int8_t wheel);
        void PressMouseButton(esp32_ps2dev::PS2Mouse::Button button);
        void ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button button);
};


extern PS2DevicesClass PS2Devices;
#endif