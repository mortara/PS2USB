#include "main.h"
#include "webserver.hpp"

QueueHandle_t ps2CmdQueue;

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    
    String msg;
    String topic_str = String(topic);
    for (byte i = 0; i < length; i++) {
        char tmp = char(payload[i]);
        msg += tmp;
    }

    if(msg == "PRESS")
    {
        PS2Cmd cmd;
        if(topic_str == "homeassistant/button/PS2Adapter_Mouse/LEFT")
        {
            cmd.type = PS2CmdType::MOUSE_MOVE;
            cmd.move.x = -10; cmd.move.y = 0; cmd.move.wheel = 0;
            ps2Enqueue(cmd);
        }
        else if(topic_str == "homeassistant/button/PS2Adapter_Mouse/RIGHT")
        {
            cmd.type = PS2CmdType::MOUSE_MOVE;
            cmd.move.x = 10; cmd.move.y = 0; cmd.move.wheel = 0;
            ps2Enqueue(cmd);
        }
        else if(topic_str == "homeassistant/button/PS2Adapter_Mouse/UP")
        {
            cmd.type = PS2CmdType::MOUSE_MOVE;
            cmd.move.x = 0; cmd.move.y = 10; cmd.move.wheel = 0;
            ps2Enqueue(cmd);
        }
        else if(topic_str == "homeassistant/button/PS2Adapter_Mouse/DOWN")
        {
            cmd.type = PS2CmdType::MOUSE_MOVE;
            cmd.move.x = 0; cmd.move.y = -10; cmd.move.wheel = 0;
            ps2Enqueue(cmd);
        }
        else if(topic_str == "homeassistant/button/PS2Adapter_Adapter/INFO")
        {
            MyEspUsbHost.DisplayInfo();
        }
        else if(topic_str == "homeassistant/button/PS2Adapter_Adapter/REBOOT")
        {
            ESP.restart();
        }
    } else if(topic_str == "homeassistant/text/PS2Adapter_Keyboard/INPUT")
    {
        PS2Cmd cmd;
        cmd.type = PS2CmdType::TYPE_TEXT;
        strncpy(cmd.text, msg.c_str(), sizeof(cmd.text) - 1);
        cmd.text[sizeof(cmd.text) - 1] = '\0';
        ps2Enqueue(cmd);
    }

    pmLogging.LogLn(topic_str + ":" + msg);
}

void setup() {

    ps2CmdQueue = xQueueCreate(16, sizeof(PS2Cmd));

    Serial.begin(115200);
    Serial.println("Hello, world!");

    Serial.println(ESP.getSdkVersion());

    pinMode(SWITCH_PINS[0], INPUT_PULLUP);
    pinMode(SWITCH_PINS[1], INPUT_PULLUP);
    pinMode(SWITCH_PINS[2], INPUT_PULLUP);
    pinMode(SWITCH_PINS[3], INPUT_PULLUP);

    int state0 = digitalRead(SWITCH_PINS[0]);
    int state1 = digitalRead(SWITCH_PINS[1]);
    int state2 = digitalRead(SWITCH_PINS[2]);
    int state3 = digitalRead(SWITCH_PINS[3]);

    Serial.printf("DIP switches: %d %d %d %d \r\n", state0, state1, state2, state3);
   
    if (state0 == ON)
    {
        pmCommonLib.Setup(false,false,true,false);
        Serial.println("Entering OTA only mode!");
        pmCommonLib.OTAHandler.OTAOnly = true;
        pmCommonLib.Start();
        return;
    }

    if(state1 == ON)
    {
        Serial.println("Swapping PS2 cables");
        PS2Devices.CablesSwapped = true;
    }

    if(state2 == ON)
    {
        Serial.println("Disabling secondary features!");
        disable_secondary_features = true;
    }

    xTaskCreatePinnedToCore(
        Task2code, /* Function to implement the task */
        "Task2", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        5,  /* Priority of the task */
        &Task2,  /* Task handle. */
        1); /* Core where the task should run */

    if(disable_secondary_features)
        return;
    
    Serial.println("Starting secondary features task...");
    xTaskCreatePinnedToCore(
    Task1code, /* Function to implement the task */
    "Task1", /* Name of the task */
    10000,  /* Stack size in words */
    NULL,  /* Task input parameter */
    0,  /* Priority of the task */
    &Task1,  /* Task handle. */
    0); /* Core where the task should run */
    
}

/// @brief This tasks executes the secondary features of the adapter: OTA, WebServer and WebSerial
/// @param parameter 
void Task1code( void * parameter) {

    pmCommonLib.Setup();
    pmCommonLib.MQTTConnector.ConfigureDevice("PS2Adapter", "MK1", "Patrick Mortara");
    MyEspUsbHost.LoadMouseSettings();
    PS2Cmd mouseTimingCmd;
    mouseTimingCmd.type = PS2CmdType::SET_MOUSE_TIMING;
    mouseTimingCmd.mouseTiming.clockHalfMicros = MyEspUsbHost.mouseClockHalfMicros;
    mouseTimingCmd.mouseTiming.byteIntervalMicros = MyEspUsbHost.mouseByteIntervalMicros;
    ps2Enqueue(mouseTimingCmd);
    pmCommonLib.Start();

    // Register after Start() so pmCommonLib's specific routes (e.g. /config) are
    // registered first and win against the "/" prefix match in ESPAsyncWebServer.
    pmCommonLib.WebServer.RegisterOn("/reboot", handleReboot, HTTP_POST);
    pmCommonLib.WebServer.RegisterOn("/ps2/keyboard", handleKeyboardEvent, HTTP_POST);
    pmCommonLib.WebServer.RegisterOn("/ps2/mouse/settings", handleMouseSettings, HTTP_POST);
    pmCommonLib.WebServer.RegisterOn("/ps2/mouse/timing", handleMouseTiming, HTTP_POST);
    pmCommonLib.WebServer.RegisterOn("/ps2/mouse", handleMouseEvent, HTTP_POST);
    pmCommonLib.WebServer.RegisterOn("/", handleRoot, HTTP_GET);

    
    for(;;) {
      delay(50);
      secondary_loop();
    }
}

/// @brief This is the main task of the adapter. It handles USB input and PS/2 output
/// @param parameter 
void Task2code( void * parameter) {

    int mouse_clock = MOUSE_CLK;
    int mouse_data = MOUSE_DATA;
    int keyboard_clock = KEYBOARD_CLK;
    int keyboard_data = KEYBOARD_DATA;

    if(PS2Devices.CablesSwapped)
    {
        Serial.println("Swapping PS/2 cable pins");
        mouse_clock = KEYBOARD_CLK;
        mouse_data = KEYBOARD_DATA;
        keyboard_clock = MOUSE_CLK;
        keyboard_data = MOUSE_DATA;
    }

    Serial.println("Initializing keyboard...");
    PS2Devices.InitKeyboard(keyboard_clock,keyboard_data); 

    // initialize mouse
    Serial.println("Initializing mouse...");  
    PS2Devices.InitMouse(mouse_clock,mouse_data);

    Serial.println("Initializing USB Host...");
    MyEspUsbHost.init();  // registers callbacks and starts the library's own internal task

    PS2Cmd cmd;
    for(;;) {
        MyEspUsbHost.CheckUsbConnectionWatchdog();
        // Block up to 10 ms waiting for the next command, then loop.
        while (xQueueReceive(ps2CmdQueue, &cmd, pdMS_TO_TICKS(10)) == pdTRUE) {
            switch (cmd.type) {
                case PS2CmdType::MOUSE_MOVE:
                    PS2Devices.MoveMouse(cmd.move.x, cmd.move.y, cmd.move.wheel);
                    break;
                case PS2CmdType::MOUSE_PRESS:
                    PS2Devices.PressMouseButton(cmd.mouseBtn.button);
                    break;
                case PS2CmdType::MOUSE_RELEASE:
                    PS2Devices.ReleaseMouseButton(cmd.mouseBtn.button);
                    break;
                case PS2CmdType::MOUSE_CLICK:
                    PS2Devices.ClickMouseButton(cmd.mouseBtn.button);
                    break;
                case PS2CmdType::MOUSE_DOUBLE_CLICK:
                    PS2Devices.DoubleClickMouseButton(cmd.mouseBtn.button);
                    break;
                case PS2CmdType::SET_MOUSE_TIMING:
                    PS2Devices.SetMouseTiming(cmd.mouseTiming.clockHalfMicros, cmd.mouseTiming.byteIntervalMicros);
                    break;
                case PS2CmdType::KEY_DOWN:
                    PS2Devices.KeyDown(cmd.key.key);
                    break;
                case PS2CmdType::KEY_UP:
                    PS2Devices.KeyUp(cmd.key.key);
                    break;
                case PS2CmdType::TYPE_TEXT:
                    PS2Devices.Type(cmd.text);
                    break;
            }
        }
    }
}

unsigned long _lastLoop = millis();

unsigned long _loopTimeMax = 0;
unsigned long _loopTimeMin = 9999999;
unsigned long _loopTimeSum = 0;
int _loopTimeCount = 0;
int _loopsToCount = 1000;

void DisplayLoopTime()
{
    unsigned long now = millis();
    unsigned long looptime = now - _lastLoop;
    _lastLoop = now;

    if(looptime > _loopTimeMax)
      _loopTimeMax = looptime;

    if(looptime < _loopTimeMin)
      _loopTimeMin = looptime;

    _loopTimeSum += looptime;
    _loopTimeCount++;

    if(_loopTimeCount == _loopsToCount)
    {
      float looptimeaverage = (float)_loopTimeSum / (float)_loopTimeCount;
      Serial.printf("Looptime: Avg: %f   Min: %u  Max: %u \r\n", looptimeaverage, _loopTimeMin, _loopTimeMax);

      _loopTimeCount = 0;
      _loopTimeSum = 0;
      _loopTimeMax = 0;
      _loopTimeMin = 9999999;
    }

}

void secondary_loop()
{
    pmCommonLib.Loop();
    

    if(pmCommonLib.OTAHandler.OTAOnly)
    {
        return;
    }

    if(WiFi.isConnected() && pmCommonLib.MQTTConnector.isActive() && !mqtt_setup)
    {
        pmCommonLib.MQTTConnector.SetupButton("REBOOT", "Adapter","","");
        pmCommonLib.MQTTConnector.SetupButton("LEFT", "Mouse","","");
        pmCommonLib.MQTTConnector.SetupButton("RIGHT", "Mouse","","");
        pmCommonLib.MQTTConnector.SetupButton("UP", "Mouse","","");
        pmCommonLib.MQTTConnector.SetupButton("DOWN", "Mouse","","");
        pmCommonLib.MQTTConnector.SetupButton("INFO", "Adapter","","");
        pmCommonLib.MQTTConnector.SetupText("INPUT", "Keyboard","","");
        mqtt_setup = true;
    }
    
}

void DebugKeys()
{
  char ch = -1;
  
  if (Serial.available()>0)
  {
      ch = Serial.read();
  }
  else
  {
      ch = pmCommonLib.WebSerial.GetInput();
  }

  switch(ch)
  {
      case (char)-1:
          break;
      case 'i':
          MyEspUsbHost.DisplayInfo();
          break;
      case 'I':
          pmLogging.LogLn(WiFi.localIP().toString());
          break;
      case 'w':
          PS2Devices.MoveMouse(0, 10, 0);
          pmLogging.LogLn("w");
          break;
      case 's':
          PS2Devices.MoveMouse(0, -10, 0);
          pmLogging.LogLn("s");
          break;
      case 'a':  
          PS2Devices.MoveMouse(-10, 0, 0);
          pmLogging.LogLn("a");
          break;
      case 'd':  
          PS2Devices.MoveMouse(10, 0, 0);
          pmLogging.LogLn("d");
          break;
  }
}

void loop() 
{
    if(pmCommonLib.OTAHandler.OTAOnly)
    {
        //Serial.println("OTA only mode!");
        secondary_loop();
        return;
    }

    DebugKeys();
}
