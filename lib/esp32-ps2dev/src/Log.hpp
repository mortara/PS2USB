#ifndef E5F7EA5A_DD85_4D90_8E52_E41100031A25
#define E5F7EA5A_DD85_4D90_8E52_E41100031A25

#include <Arduino.h>
#include <string>

#define PS2DEV_LOG_LEVEL_NONE 0
#define PS2DEV_LOG_LEVEL_ERROR 1
#define PS2DEV_LOG_LEVEL_WARN 2
#define PS2DEV_LOG_LEVEL_INFO 3
#define PS2DEV_LOG_LEVEL_DEBUG 4
#define PS2DEV_LOG_LEVEL_VERBOSE 5

#ifndef PS2DEV_LOG_LEVEL
#define PS2DEV_LOG_LEVEL PS2DEV_LOG_LEVEL_NONE
#endif

#ifndef PS2DEV_LOG_SERIAL
#define PS2DEV_LOG_SERIAL (Serial)
#endif

#ifndef PS2DEV_LOG_TASK_PRIORITY
#define PS2DEV_LOG_TASK_PRIORITY 0
#endif

#ifndef PS2DEV_LOG_TASK_CORE
#define PS2DEV_LOG_TASK_CORE APP_CPU_NUM
#endif

#ifndef PS2DEV_LOG_QUEUE_SIZE
#define PS2DEV_LOG_QUEUE_SIZE 20
#endif

#define PS2DEV_LOG_START()                                                                                                            \
  do {                                                                                                                                \
    ::esp32_ps2dev::xQueueSerialOutput = xQueueCreate(PS2DEV_LOG_QUEUE_SIZE, sizeof(std::string*));                                   \
    xTaskCreateUniversal(::esp32_ps2dev::taskSerial, "PS2DEV_LOG", 4096, NULL, PS2DEV_LOG_TASK_PRIORITY, NULL, PS2DEV_LOG_TASK_CORE); \
  } while (0)

// clang-format off
#define PS2DEV_LOG(str) ::esp32_ps2dev::serialPrint(str)
#define PS2DEV_LOGE(str) do {} while (0)
#define PS2DEV_LOGW(str) do {} while (0)
#define PS2DEV_LOGI(str) do {} while (0)
#define PS2DEV_LOGD(str) do {} while (0)
#define PS2DEV_LOGV(str) do {} while (0)
// clang-format on

#if PS2DEV_LOG_LEVEL >= PS2DEV_LOG_LEVEL_ERROR
#undef PS2DEV_LOGE
#define PS2DEV_LOGE(str) ::esp32_ps2dev::serialPrintln(str)
#endif
#if PS2DEV_LOG_LEVEL >= PS2DEV_LOG_LEVEL_WARN
#undef PS2DEV_LOGW
#define PS2DEV_LOGW(str) ::esp32_ps2dev::serialPrintln(str)
#endif
#if PS2DEV_LOG_LEVEL >= PS2DEV_LOG_LEVEL_INFO
#undef PS2DEV_LOGI
#define PS2DEV_LOGI(str) ::esp32_ps2dev::serialPrintln(str)
#endif
#if PS2DEV_LOG_LEVEL >= PS2DEV_LOG_LEVEL_DEBUG
#undef PS2DEV_LOGD
#define PS2DEV_LOGD(str) ::esp32_ps2dev::serialPrintln(str)
#endif
#if PS2DEV_LOG_LEVEL >= PS2DEV_LOG_LEVEL_VERBOSE
#undef PS2DEV_LOGV
#define PS2DEV_LOGV(str) ::esp32_ps2dev::serialPrintln(str)
#endif

namespace esp32_ps2dev {

void serialPrint(const std::string& str);
void serialPrint(const char* str);
void serialPrint(char c);
void serialPrintln(const std::string& str);
void serialPrintln(const char* str);
void serialPrintln(char c);
void serialPrintln(void);
void taskSerial(void* arg);

extern QueueHandle_t xQueueSerialOutput;

}  // namespace esp32_ps2dev

#endif /* E5F7EA5A_DD85_4D90_8E52_E41100031A25 */
