#include "PS2Mouse.hpp"

namespace esp32_ps2dev {

const uint32_t MOUSE_CLICK_PRESSING_DURATION_MILLIS = 100;

PS2Mouse::PS2Mouse(int clk, int data) : PS2dev(clk, data) {}
void PS2Mouse::begin(bool restore_internal_state) {
  PS2dev::begin();

  auto ret = nvs_flash_init();
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::begin: nvs_flash_init failed");
  }
  const auto nvs_ns = std::string("ps2dev") + std::to_string(_ps2clk) + std::to_string(_ps2data);
  ret = nvs_open(nvs_ns.c_str(), NVS_READWRITE, &_nvs_handle);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::begin: nvs_open failed");
  }

  if (!restore_internal_state) {
    xSemaphoreTake(_mutex_bus, portMAX_DELAY);
    write(0xAA);
    delayMicroseconds(_config_byte_interval_micros);
    write(0x00);
    xSemaphoreGive(_mutex_bus);
  } else {
    _load_internal_state_from_nvs();
  }

  xTaskCreateUniversal(_taskfn_poll_mouse_count, "PS2Mouse", 4096, this, _config_task_priority - 1, &_task_poll_mouse_count,
                       _config_task_core);
}

int PS2Mouse::reply_to_host(uint8_t host_cmd) {
  uint8_t val;
  if (_mode == Mode::WRAP_MODE) {
    switch ((Command)host_cmd) {
      case Command::SET_WRAP_MODE:  // set wrap mode
        PS2DEV_LOGD("PS2Mouse::reply_to_host: (WRAP_MODE) Set wrap mode command received");
        ack();
        reset_counter();
        break;
      case Command::RESET_WRAP_MODE:  // reset wrap mode
        PS2DEV_LOGD("PS2Mouse::reply_to_host: (WRAP_MODE) Reset wrap mode command received");
        ack();
        reset_counter();
        _mode = _last_mode;
        _save_internal_state_to_nvs();
        break;
      default:
        write(host_cmd);
    }
    return 0;
  }

  switch ((Command)host_cmd) {
    case Command::RESET:  // reset
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Reset command received");
      ack();
      // the while loop lets us wait for the host to be ready
      while (write(0xAA) != 0) delay(1);
      delayMicroseconds(_config_byte_interval_micros);
      while (write(0x00) != 0) delay(1);
      delayMicroseconds(_config_byte_interval_micros);
      _has_wheel = false;
      _has_4th_and_5th_buttons = false;
      _sample_rate = 100;
      _resolution = ResolutionCode::RES_4;
      _scale = Scale::ONE_ONE;
      _data_reporting_enabled = false;
      _mode = Mode::STREAM_MODE;
      _save_internal_state_to_nvs();
      reset_counter();
      break;
    case Command::RESEND:  // resend
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Resend command received");
      ack();
      break;
    case Command::SET_DEFAULTS:  // set defaults
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Set defaults command received");
      // enter stream mode
      ack();
      _sample_rate = 100;
      _resolution = ResolutionCode::RES_4;
      _scale = Scale::ONE_ONE;
      _data_reporting_enabled = false;
      _mode = Mode::STREAM_MODE;
      _save_internal_state_to_nvs();
      reset_counter();
      break;
    case Command::DISABLE_DATA_REPORTING:  // disable data reporting
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Disable data reporting command received");
      ack();
      _data_reporting_enabled = false;
      _save_internal_state_to_nvs();
      reset_counter();
      break;
    case Command::ENABLE_DATA_REPORTING:  // enable data reporting
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Enable data reporting command received");
      ack();
      _data_reporting_enabled = true;
      _save_internal_state_to_nvs();
      reset_counter();
      break;
    case Command::SET_SAMPLE_RATE:  // set sample rate
      ack();
      if (read(&val) == 0) {
        switch (val) {
          case 10:
          case 20:
          case 40:
          case 60:
          case 80:
          case 100:
          case 200:
            _sample_rate = val;
            _last_sample_rate[0] = _last_sample_rate[1];
            _last_sample_rate[1] = _last_sample_rate[2];
            _last_sample_rate[2] = val;
            PS2DEV_LOGD(std::string("Set sample rate command received: ") + String(val).c_str());
            ack();
            break;

          default:
            break;
        }
        _save_internal_state_to_nvs();
        // _min_report_interval_us = 1000000 / sample_rate;
        reset_counter();
      }
      break;
    case Command::GET_DEVICE_ID:  // get device id
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Get device id command received");
      ack();
      if (_last_sample_rate[0] == 200 && _last_sample_rate[1] == 100 && _last_sample_rate[2] == 80) {
        write(0x03);  // Intellimouse with wheel
        delayMicroseconds(_config_byte_interval_micros);
        PS2DEV_LOGD("PS2Mouse::reply_to_host: Act as Intellimouse with wheel.");
        _has_wheel = true;
        _save_internal_state_to_nvs();
      } else if (_last_sample_rate[0] == 200 && _last_sample_rate[1] == 200 && _last_sample_rate[2] == 80 && _has_wheel == true) {
        write(0x04);  // Intellimouse with 4th and 5th buttons
        delayMicroseconds(_config_byte_interval_micros);
        PS2DEV_LOGD("PS2Mouse::reply_to_host: Act as Intellimouse with 4th and 5th buttons.");
        _has_4th_and_5th_buttons = true;
        _save_internal_state_to_nvs();
      } else {
        write(0x00);  // Standard PS/2 mouse
        delayMicroseconds(_config_byte_interval_micros);
        PS2DEV_LOGD("PS2Mouse::reply_to_host: Act as standard PS/2 mouse.");
        _has_wheel = false;
        _has_4th_and_5th_buttons = false;
        _save_internal_state_to_nvs();
      }
      reset_counter();
      break;
    case Command::SET_REMOTE_MODE:  // set remote mode
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Set remote mode command received");
      ack();
      reset_counter();
      _mode = Mode::REMOTE_MODE;
      _save_internal_state_to_nvs();
      break;
    case Command::SET_WRAP_MODE:  // set wrap mode
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Set wrap mode command received");
      ack();
      reset_counter();
      _last_mode = _mode;
      _mode = Mode::WRAP_MODE;
      _save_internal_state_to_nvs();
      break;
    case Command::RESET_WRAP_MODE:  // reset wrap mode
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Reset wrap mode command received");
      ack();
      reset_counter();
      break;
    case Command::READ_DATA:  // read data
      ack();
      send_packet_to_queue(get_packet());
      reset_counter();
      break;
    case Command::SET_STREAM_MODE:  // set stream mode
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Set stream mode command received");
      ack();
      reset_counter();
      break;
    case Command::STATUS_REQUEST:  // status request
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Status request command received");
      ack();
      _send_status();
      break;
    case Command::SET_RESOLUTION:  // set resolution
      ack();
      if (read(&val) == 0 && val <= 3) {
        _resolution = (ResolutionCode)val;
        PS2DEV_LOGD(std::string("PS2Mouse::reply_to_host: Set resolution command received: ") + String(val, HEX).c_str());
        ack();
        _save_internal_state_to_nvs();
        reset_counter();
      }
      break;
    case Command::SET_SCALING_2_1:  // set scaling 2:1
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Set scaling 2:1 command received");
      ack();
      _scale = Scale::TWO_ONE;
      _save_internal_state_to_nvs();
      break;
    case Command::SET_SCALING_1_1:  // set scaling 1:1
      PS2DEV_LOGD("PS2Mouse::reply_to_host: Set scaling 1:1 command received");
      ack();
      _scale = Scale::ONE_ONE;
      _save_internal_state_to_nvs();
      break;
    default:
      PS2DEV_LOGD(std::string("PS2Mouse::reply_to_host: Unknown command received: ") + String(host_cmd, HEX).c_str());
      break;
  }
  return 0;
}

bool PS2Mouse::has_wheel() { return _has_wheel; }
bool PS2Mouse::has_4th_and_5th_buttons() { return _has_4th_and_5th_buttons; }
bool PS2Mouse::data_reporting_enabled() { return _data_reporting_enabled; }

void PS2Mouse::reset_counter() {
  taskENTER_CRITICAL(&_mutex_counter);
  _count_x = 0;
  _count_y = 0;
  _count_z = 0;
  _count_x_overflow = 0;
  _count_y_overflow = 0;
  _count_or_button_changed = false;
  taskEXIT_CRITICAL(&_mutex_counter);
}

uint8_t PS2Mouse::get_sample_rate() { return _sample_rate; }
uint8_t PS2Mouse::get_resolution_code() { return (uint8_t)_resolution; }
uint8_t PS2Mouse::get_scale_code() { return (uint8_t)_scale; }
uint8_t PS2Mouse::get_mode_code() { return (uint8_t)_mode; }
void PS2Mouse::move(int16_t x, int16_t y, int8_t wheel) {
  taskENTER_CRITICAL(&_mutex_counter);
  _count_x += x;
  _count_y += y;
  _count_z += wheel;
  _count_or_button_changed = true;
  taskEXIT_CRITICAL(&_mutex_counter);
}

void PS2Mouse::press(Button button) {
  bool left;
  bool right;
  bool middle;
  bool button_4;
  bool button_5;
  taskENTER_CRITICAL(&_mutex_counter);
  switch (button) {
    case Button::LEFT:
      _button_left = 1;
      break;
    case Button::RIGHT:
      _button_right = 1;
      break;
    case Button::MIDDLE:
      _button_middle = 1;
      break;
    case Button::BUTTON_4:
      _button_4th = 1;
      break;
    case Button::BUTTON_5:
      _button_5th = 1;
      break;
    default:
      break;
  }
  _count_or_button_changed = true;
  left = _button_left;
  right = _button_right;
  middle = _button_middle;
  button_4 = _button_4th;
  button_5 = _button_5th;
  taskEXIT_CRITICAL(&_mutex_counter);
  send_report(0, 0, 0, left, right, middle, button_4, button_5);
}

void PS2Mouse::release(Button button) {
  bool left;
  bool right;
  bool middle;
  bool button_4;
  bool button_5;
  taskENTER_CRITICAL(&_mutex_counter);
  switch (button) {
    case Button::LEFT:
      _button_left = 0;
      break;
    case Button::RIGHT:
      _button_right = 0;
      break;
    case Button::MIDDLE:
      _button_middle = 0;
      break;
    case Button::BUTTON_4:
      _button_4th = 0;
      break;
    case Button::BUTTON_5:
      _button_5th = 0;
      break;
    default:
      break;
  }
  _count_or_button_changed = true;
  left = _button_left;
  right = _button_right;
  middle = _button_middle;
  button_4 = _button_4th;
  button_5 = _button_5th;
  taskEXIT_CRITICAL(&_mutex_counter);
  send_report(0, 0, 0, left, right, middle, button_4, button_5);
}

void PS2Mouse::click(Button button) {
  press(button);
  delay(MOUSE_CLICK_PRESSING_DURATION_MILLIS);
  release(button);
}

void PS2Mouse::move_and_buttons(int16_t x, int16_t y, int8_t wheel, bool left, bool right, bool middle, bool button_4, bool button_5) {
  taskENTER_CRITICAL(&_mutex_counter);
  _count_x += x;
  _count_y += y;
  _count_z += wheel;
  _button_left = left ? 1 : 0;
  _button_right = right ? 1 : 0;
  _button_middle = middle ? 1 : 0;
  _button_4th = button_4 ? 1 : 0;
  _button_5th = button_5 ? 1 : 0;
  _count_or_button_changed = true;
  taskEXIT_CRITICAL(&_mutex_counter);
}

bool PS2Mouse::is_count_or_button_changed() {
  taskENTER_CRITICAL(&_mutex_counter);
  bool changed = _count_or_button_changed;
  taskEXIT_CRITICAL(&_mutex_counter);
  return changed;
}

PS2Packet PS2Mouse::make_packet(int16_t x, int16_t y, int8_t wheel, bool left, bool right, bool middle, bool button_4, bool button_5) {
  PS2Packet packet;
  // if scale is 2:1, we need to adjust the values
  //   Movement Counter  Reported Movement
  //         0                0
  //         1                1
  //         2                1
  //         3                3
  //         4                6
  //         5                9
  //         N > 5            2 * N
  if (_scale == Scale::TWO_ONE) {
    int16_t* p[2] = {&x, &y};
    for (size_t i = 0; i < 2; i++) {
      boolean positive = *p[i] >= 0;
      uint16_t abs_value = positive ? *p[i] : -*p[i];
      switch (abs_value) {
        case 1:
          abs_value = 1;
          break;
        case 2:
          abs_value = 1;
          break;
        case 3:
          abs_value = 3;
          break;
        case 4:
          abs_value = 6;
          break;
        case 5:
          abs_value = 9;
          break;
        default:
          abs_value *= 2;
          break;
      }
      if (!positive) *p[i] = -abs_value;
    }
  }
  // set overflow flags if necessary
  auto x_overflow = 0;
  auto y_overflow = 0;
  if (x > 255) {
    x_overflow = 1;
    x = 255;
  } else if (x < -255) {
    x_overflow = 1;
    x = -255;
  }
  if (y > 255) {
    y_overflow = 1;
    y = 255;
  } else if (y < -255) {
    y_overflow = 1;
    y = -255;
  }
  if (wheel > 7) {
    wheel = 7;
  } else if (wheel < -8) {
    wheel = -8;
  }
  // build the packet
  packet.data[0] =
      (left) | ((right) << 1) | ((middle) << 2) | (1 << 3) | ((x < 0) << 4) | ((y < 0) << 5) | (x_overflow << 6) | (y_overflow << 7);
  packet.data[1] = x & 0xFF;
  packet.data[2] = y & 0xFF;
  if (_has_wheel) {
    packet.len = 4;
    if (_has_4th_and_5th_buttons) {
      // if Intellimouse with wheel and with 4th and 5th buttons,
      // the first 4 bits of the 4th byte is the wheel counter,
      // and the 5th and 6th bits are the 4th and 5th buttons
      packet.data[3] = (wheel & 0x0F) | ((button_4) << 4) | ((button_5) << 5);
    } else {
      // if Intellimouse with wheel but without 4th and 5th buttons,
      // the 4th byte is the wheel counter
      packet.data[3] = wheel & 0xFF;
    }
  } else {
    packet.len = 3;
  }
  return packet;
}

PS2Packet PS2Mouse::get_packet() {
  taskENTER_CRITICAL(&_mutex_counter);
  PS2Packet packet = make_packet(_count_x, _count_y, _count_z, _button_left, _button_right, _button_middle, _button_4th, _button_5th);
  taskEXIT_CRITICAL(&_mutex_counter);
  return packet;
}

PS2Packet PS2Mouse::get_packet_and_reset_counter() {
  taskENTER_CRITICAL(&_mutex_counter);
  PS2Packet packet = make_packet(_count_x, _count_y, _count_z, _button_left, _button_right, _button_middle, _button_4th, _button_5th);
  _count_x = 0;
  _count_y = 0;
  _count_z = 0;
  _count_x_overflow = 0;
  _count_y_overflow = 0;
  _count_or_button_changed = false;
  taskEXIT_CRITICAL(&_mutex_counter);
  return packet;
}

// Send a report to the host immediately.
// Use with care, this function ignore the sample rate specified by the host.
void IRAM_ATTR PS2Mouse::send_report(int16_t x, int16_t y, int8_t wheel, bool left, bool right, bool middle, bool button_4, bool button_5) {
  PS2Packet packet = make_packet(x, y, wheel, left, right, middle, button_4, button_5);
  if (_data_reporting_enabled) {
    send_packet_to_queue(packet);
  }
}

void PS2Mouse::_save_internal_state_to_nvs() {
  auto ret = nvs_set_u8(_nvs_handle, "hasWheel", _has_wheel);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_save_internal_state_to_nvs: nvs_set_u8 failed to save hasWheel.");
  }
  ret = nvs_set_u8(_nvs_handle, "has4and5Btn", _has_4th_and_5th_buttons);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_save_internal_state_to_nvs: nvs_set_u8 failed to save has4and5Btn.");
  }
  ret = nvs_set_u8(_nvs_handle, "dataRepEn", _data_reporting_enabled);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_save_internal_state_to_nvs: nvs_set_u8 failed to save dataRepEn.");
  }
  ret = nvs_set_u8(_nvs_handle, "resolution", (uint8_t)_resolution);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_save_internal_state_to_nvs: nvs_set_u8 failed to save resolution.");
  }
  ret = nvs_set_u8(_nvs_handle, "scale", (uint8_t)_scale);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_save_internal_state_to_nvs: nvs_set_u8 failed to save scale.");
  }
  ret = nvs_set_u8(_nvs_handle, "mode", (uint8_t)_mode);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_save_internal_state_to_nvs: nvs_set_u8 failed to save mode.");
  }
}

void PS2Mouse::_load_internal_state_from_nvs() {
  auto ret = nvs_get_u8(_nvs_handle, "hasWheel", (uint8_t*)&_has_wheel);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_load_internal_state_from_nvs: nvs_get_u8 failed to load hasWheel.");
  }
  nvs_get_u8(_nvs_handle, "has4and5Btn", (uint8_t*)&_has_4th_and_5th_buttons);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_load_internal_state_from_nvs: nvs_get_u8 failed to load has4and5Btn.");
  }
  nvs_get_u8(_nvs_handle, "dataRepEn", (uint8_t*)&_data_reporting_enabled);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_load_internal_state_from_nvs: nvs_get_u8 failed to load dataRepEn.");
  }
  nvs_get_u8(_nvs_handle, "resolution", (uint8_t*)&_resolution);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_load_internal_state_from_nvs: nvs_get_u8 failed to load resolution.");
  }
  nvs_get_u8(_nvs_handle, "scale", (uint8_t*)&_scale);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_load_internal_state_from_nvs: nvs_get_u8 failed to load scale.");
  }
  nvs_get_u8(_nvs_handle, "mode", (uint8_t*)&_mode);
  if (ret != ESP_OK) {
    PS2DEV_LOGE("PS2Mouse::_load_internal_state_from_nvs: nvs_get_u8 failed to load mode.");
  }
}

void PS2Mouse::_send_status() {
  PS2Packet packet;
  packet.len = 3;
  boolean mode = (_mode == Mode::REMOTE_MODE);
  packet.data[0] = (_button_right & 1) | ((_button_middle & 1) << 1) | ((_button_left & 1) << 2) |
                   (((uint8_t)_scale & 1) << 4) | ((_data_reporting_enabled & 1) << 5) | ((mode & 1) << 6);
  packet.data[1] = (uint8_t)_resolution;
  packet.data[2] = _sample_rate;
  send_packet_to_queue(packet);
}

void _taskfn_poll_mouse_count(void* arg) {
  PS2Mouse* ps2mouse = (PS2Mouse*)arg;
  while (true) {
    if (ps2mouse->data_reporting_enabled() && ps2mouse->is_count_or_button_changed()) {
      ps2mouse->send_packet_to_queue(ps2mouse->get_packet_and_reset_counter());
    }
    uint8_t sample_rate = ps2mouse->get_sample_rate();
    uint32_t report_interval_ms = sample_rate ? (1000 / sample_rate) : 10;
    delay(report_interval_ms);
  }
  vTaskDelete(NULL);
}

}  // namespace esp32_ps2dev
