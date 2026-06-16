#include <PS2Mouse.hpp>
#include <PS2Keyboard.hpp>

#ifndef PS2DEVICES_H
#define PS2DEVICES_H

class PS2DevicesClass
{
    private:
        esp32_ps2dev::PS2Mouse *_mouse = nullptr;
        esp32_ps2dev::PS2Keyboard *_keyboard = nullptr;
       
    public:
        bool CablesSwapped = false;

        ~PS2DevicesClass();
        void InitKeyboard(int clock, int data);
        void InitMouse(int clock, int data);

        void KeyUp(esp32_ps2dev::scancodes::Key key);
        void KeyDown(esp32_ps2dev::scancodes::Key key);

        void Type(esp32_ps2dev::scancodes::Key key);
        void Type(const char *text);
        void Type(char ch);
        uint8_t GetMouseSampleRate();
        uint16_t GetMouseReportIntervalMs();
        uint8_t GetMouseResolutionCode();
        uint8_t GetMouseScaleCode();
        uint8_t GetMouseModeCode();
        bool IsMouseDataReportingEnabled();
        uint16_t GetMouseClockHalfPeriodMicros();
        uint16_t GetMouseByteIntervalMicros();
        uint16_t GetMouseClickPressMillis();
        uint16_t GetMouseDoubleClickGapMillis();
        void SetMouseTiming(uint16_t clockHalfMicros, uint16_t byteIntervalMicros);
        void MoveMouse(int16_t x, int16_t y, int8_t wheel);
        void PressMouseButton(esp32_ps2dev::PS2Mouse::Button button);
        void ReleaseMouseButton(esp32_ps2dev::PS2Mouse::Button button);
        void ClickMouseButton(esp32_ps2dev::PS2Mouse::Button button);
        void DoubleClickMouseButton(esp32_ps2dev::PS2Mouse::Button button);
};


extern PS2DevicesClass PS2Devices;
#endif
