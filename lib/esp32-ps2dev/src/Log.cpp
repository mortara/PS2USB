#include "Log.hpp"

namespace esp32_ps2dev {

QueueHandle_t xQueueSerialOutput;

void serialPrint(const std::string& str) {
  auto strCopy = new std::string(str);
  xQueueSend(xQueueSerialOutput, &strCopy, portMAX_DELAY);
}
void serialPrint(const char* str) { serialPrint(std::string(str)); }
void serialPrint(char c) { serialPrint(std::string(1, c)); }
void serialPrintln(void) { serialPrint("\r\n"); }
void serialPrintln(const std::string& str) { serialPrint(str + "\r\n"); }
void serialPrintln(const char* str) { serialPrint(std::string(str) + "\r\n"); }
void serialPrintln(char c) { serialPrint(std::string(1, c) + "\r\n"); }

void taskSerial(void* arg) {
  while (true) {
    std::string* str;
    if (xQueueReceive(xQueueSerialOutput, &str, portMAX_DELAY) == pdTRUE) {
      PS2DEV_LOG_SERIAL.print(str->c_str());
      delete str;
    }
  }
  vTaskDelete(NULL);
}

}  // namespace esp32_ps2dev
