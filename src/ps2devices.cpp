#include "ps2devices.h"

void PS2DevicesClass::InitKeyboard(int clock, int data)
{
    PS2Devices._keyboard = new esp32_ps2dev::PS2Keyboard(clock, data);
}


void PS2DevicesClass::InitMouse(int clock, int data)
{
    PS2Devices._mouse = new esp32_ps2dev::PS2Mouse(clock, data);
}

void PS2DevicesClass::Type(esp32_ps2dev::scancodes::Key key)
{
    _keyboard->type(key);
}

void PS2DevicesClass::Type(const char *text)
{
    _keyboard->type(text);
}

void PS2DevicesClass::MoveMouse(int x, int y, int wheel)
{
    _mouse->move(x, y, wheel);
}

void PS2DevicesClass::PressMouse(esp32_ps2dev::PS2Mouse::Button button)
{
    _mouse->press(button);
}

PS2DevicesClass PS2Devices;