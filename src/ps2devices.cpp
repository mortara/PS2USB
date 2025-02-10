#include "ps2devices.h"

void PS2DevicesClass::InitKeyboard(int clock, int data)
{
    pinMode(clock, OUTPUT);
    pinMode(data, OUTPUT);

    _keyboard = new esp32_ps2dev::PS2Keyboard(clock, data);
    _keyboard->begin();
}


void PS2DevicesClass::InitMouse(int clock, int data)
{
    pinMode(clock, OUTPUT);
    pinMode(data, OUTPUT);
    _mouse = new esp32_ps2dev::PS2Mouse(clock, data);
    _mouse->begin();
}

void PS2DevicesClass::KeyDown(esp32_ps2dev::scancodes::Key key)
{
    _keyboard->keydown(key);
}

void PS2DevicesClass::KeyUp(esp32_ps2dev::scancodes::Key key)
{
    _keyboard->keyup(key);
}

void PS2DevicesClass::Type(esp32_ps2dev::scancodes::Key key)
{
    _keyboard->type(key);
}

void PS2DevicesClass::Type(const char *text)
{
    _keyboard->type(text);
}

void PS2DevicesClass::Type(char ch)
{
    _keyboard->type(String(ch).c_str());
}

void PS2DevicesClass::MoveMouse(int16_t x, int16_t y, int8_t wheel)
{
    _mouse->move(x, y, wheel);
}

void PS2DevicesClass::PressMouseButton(esp32_ps2dev::PS2Mouse::Button button)
{
    _mouse->press(button);
}

void PS2DevicesClass::ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button button)
{
    _mouse->release(button);
}

PS2DevicesClass PS2Devices;