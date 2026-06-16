#include "ps2devices.h"

static const uint16_t DEFAULT_MOUSE_CLK_HALF_MICROS = 50;
static const uint16_t DEFAULT_MOUSE_BYTE_INTERVAL_MICROS = 500;
static const uint16_t MOUSE_CLICK_PRESS_MILLIS = 70;
static const uint16_t MOUSE_DOUBLE_CLICK_GAP_MILLIS = 120;

PS2DevicesClass::~PS2DevicesClass()
{
    if (_mouse != nullptr) {
        delete _mouse;
        _mouse = nullptr;
    }
    if (_keyboard != nullptr) {
        delete _keyboard;
        _keyboard = nullptr;
    }
}

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
    _mouse->set_clk_half_period_micros(DEFAULT_MOUSE_CLK_HALF_MICROS);
    _mouse->set_byte_interval_micros(DEFAULT_MOUSE_BYTE_INTERVAL_MICROS);
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

uint8_t PS2DevicesClass::GetMouseSampleRate()
{
    return _mouse ? _mouse->get_sample_rate() : 0;
}

uint16_t PS2DevicesClass::GetMouseReportIntervalMs()
{
    uint8_t sampleRate = GetMouseSampleRate();
    return sampleRate ? (1000 / sampleRate) : 0;
}

uint8_t PS2DevicesClass::GetMouseResolutionCode()
{
    return _mouse ? _mouse->get_resolution_code() : 0;
}

uint8_t PS2DevicesClass::GetMouseScaleCode()
{
    return _mouse ? _mouse->get_scale_code() : 0;
}

uint8_t PS2DevicesClass::GetMouseModeCode()
{
    return _mouse ? _mouse->get_mode_code() : 0;
}

bool PS2DevicesClass::IsMouseDataReportingEnabled()
{
    return _mouse ? _mouse->data_reporting_enabled() : false;
}

uint16_t PS2DevicesClass::GetMouseClockHalfPeriodMicros()
{
    return _mouse ? _mouse->get_clk_half_period_micros() : DEFAULT_MOUSE_CLK_HALF_MICROS;
}

uint16_t PS2DevicesClass::GetMouseByteIntervalMicros()
{
    return _mouse ? _mouse->get_byte_interval_micros() : DEFAULT_MOUSE_BYTE_INTERVAL_MICROS;
}

uint16_t PS2DevicesClass::GetMouseClickPressMillis()
{
    return MOUSE_CLICK_PRESS_MILLIS;
}

uint16_t PS2DevicesClass::GetMouseDoubleClickGapMillis()
{
    return MOUSE_DOUBLE_CLICK_GAP_MILLIS;
}

void PS2DevicesClass::SetMouseTiming(uint16_t clockHalfMicros, uint16_t byteIntervalMicros)
{
    if (!_mouse) return;
    _mouse->set_clk_half_period_micros(clockHalfMicros);
    _mouse->set_byte_interval_micros(byteIntervalMicros);
    _mouse->reset_counter();
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

void PS2DevicesClass::ClickMouseButton(esp32_ps2dev::PS2Mouse::Button button)
{
    if (!_mouse) return;
    PressMouseButton(button);
    delay(MOUSE_CLICK_PRESS_MILLIS);
    ReleaseMouseButton(button);
}

void PS2DevicesClass::DoubleClickMouseButton(esp32_ps2dev::PS2Mouse::Button button)
{
    if (!_mouse) return;
    ClickMouseButton(button);
    delay(MOUSE_DOUBLE_CLICK_GAP_MILLIS);
    ClickMouseButton(button);
}

PS2DevicesClass PS2Devices;
