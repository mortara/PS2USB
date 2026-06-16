#ifndef BF34F287_08B4_47E4_9442_B0CEF4061E96
#define BF34F287_08B4_47E4_9442_B0CEF4061E96

#include <nvs_flash.h>

#include "PS2Dev.hpp"

namespace esp32_ps2dev {

class PS2Mouse : public PS2dev {
 public:
  PS2Mouse(int clk, int data);
  enum class ResolutionCode : uint8_t { RES_1 = 0x00, RES_2 = 0x01, RES_4 = 0x02, RES_8 = 0x03 };
  enum class Scale : uint8_t { ONE_ONE = 0, TWO_ONE = 1 };
  enum class Mode : uint8_t { REMOTE_MODE = 0, STREAM_MODE = 1, WRAP_MODE = 2 };
  enum class Command : uint8_t {
    RESET = 0xFF,
    RESEND = 0xFE,
    ERROR = 0xFC,
    ACK = 0xFA,
    SET_DEFAULTS = 0xF6,
    DISABLE_DATA_REPORTING = 0xF5,
    ENABLE_DATA_REPORTING = 0xF4,
    SET_SAMPLE_RATE = 0xF3,
    GET_DEVICE_ID = 0xF2,
    SET_REMOTE_MODE = 0xF0,
    SET_WRAP_MODE = 0xEE,
    RESET_WRAP_MODE = 0xEC,
    READ_DATA = 0xEB,
    SET_STREAM_MODE = 0xEA,
    STATUS_REQUEST = 0xE9,
    SET_RESOLUTION = 0xE8,
    SET_SCALING_2_1 = 0xE7,
    SET_SCALING_1_1 = 0xE6,
    SELF_TEST_PASSED = 0xAA,
  };
  enum class Button : uint8_t {
    LEFT,
    RIGHT,
    MIDDLE,
    BUTTON_4,
    BUTTON_5,
  };

  void begin(bool restore_internal_state = false);
  int reply_to_host(uint8_t host_cmd);
  bool has_wheel();
  bool has_4th_and_5th_buttons();
  bool data_reporting_enabled();
  void reset_counter();
  uint8_t get_sample_rate();
  uint8_t get_resolution_code();
  uint8_t get_scale_code();
  uint8_t get_mode_code();
  void move(int16_t x, int16_t y, int8_t wheel);
  void press(Button button);
  void release(Button button);
  void click(Button button);
  void move_and_buttons(int16_t x, int16_t y, int8_t wheel, bool left, bool right, bool middle, bool button_4, bool button_5);
  bool is_count_or_button_changed();
  PS2Packet make_packet(int16_t x, int16_t y, int8_t wheel, bool left, bool right, bool middle, bool button_4, bool button_5);
  PS2Packet get_packet();
  PS2Packet get_packet_and_reset_counter();
  void send_report(int16_t x, int16_t y, int8_t wheel, bool left, bool right, bool middle, bool button_4, bool button_5);

 protected:
  void _send_status();
  void _save_internal_state_to_nvs();
  void _load_internal_state_from_nvs();
  TaskHandle_t _task_poll_mouse_count;
  nvs_handle _nvs_handle;
  bool _has_wheel = false;
  bool _has_4th_and_5th_buttons = false;
  bool _data_reporting_enabled = false;
  ResolutionCode _resolution = ResolutionCode::RES_4;
  Scale _scale = Scale::ONE_ONE;
  Mode _mode = Mode::STREAM_MODE;
  Mode _last_mode = Mode::STREAM_MODE;
  uint8_t _last_sample_rate[3] = {0, 0, 0};
  uint8_t _sample_rate = 100;
  int16_t _count_x = 0;
  uint8_t _count_x_overflow = 0;
  int16_t _count_y = 0;
  uint8_t _count_y_overflow = 0;
  int8_t _count_z = 0;
  uint8_t _button_left = 0;
  uint8_t _button_right = 0;
  uint8_t _button_middle = 0;
  uint8_t _button_4th = 0;
  uint8_t _button_5th = 0;
  bool _count_or_button_changed = false;
  portMUX_TYPE _mutex_counter = portMUX_INITIALIZER_UNLOCKED;
};

void _taskfn_poll_mouse_count(void* arg);

}  // namespace esp32_ps2dev

#endif /* BF34F287_08B4_47E4_9442_B0CEF4061E96 */
