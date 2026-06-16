#include <Arduino.h>
#include <PS2Keyboard.hpp>

const int CLK_PIN = 19;
const int DATA_PIN = 18;

esp32_ps2dev::PS2Keyboard keyboard(CLK_PIN, DATA_PIN);

void setup() { keyboard.begin(); }

void loop() {
  delay(1000);
  keyboard.type("Hello, world! ");
}
