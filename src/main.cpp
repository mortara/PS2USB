#include "main.h"

void setup() {

    Serial.begin(115200);
    Serial.println("Hello, world!");

    pinMode(SWITCH_PINS[0], INPUT_PULLUP);

    int state = digitalRead(SWITCH_PINS[0]);
    Serial.println(state);
    if (state == ON)
    {
        Serial.println("Entering OTA only mode!");
        OTAHandler.OTAOnly = true;

        Serial.println("Starting wifi connection...");
        WIFIManager.Connect();
        return;
    }

    xTaskCreatePinnedToCore(
        Task1code, /* Function to implement the task */
        "Task1", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        0,  /* Priority of the task */
        &Task1,  /* Task handle. */
        0); /* Core where the task should run */

    xTaskCreatePinnedToCore(
        Task2code, /* Function to implement the task */
        "Task2", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        5,  /* Priority of the task */
        &Task2,  /* Task handle. */
        1); /* Core where the task should run */

}

/// @brief This tasks executes the secondary features of the adapter: OTA, WebServer and WebSerial
/// @param parameter 
void Task1code( void * parameter) {

    Serial.println("Starting wifi connection...");
    WIFIManager.Connect();

    Serial.println("Starting webserver...");
    WebServer.Setup();

    for(;;) {
      delay(50);
      secondary_loop();
    }
}

/// @brief This is the main task of the adapter. It handles USB input and PS/2 output
/// @param parameter 
void Task2code( void * parameter) {

    Serial.println("Initializing keyboard...");
    PS2Devices.InitKeyboard(KEYBOARD_CLK,KEYBOARD_DATA); 

    // initialize mouse
    Serial.println("Initializing mouse...");  
    PS2Devices.InitMouse(MOUSE_CLK,MOUSE_DATA);

    Serial.println("Initializing USB Host...");
    MyEspUsbHost.init();

    for(;;) {
      //delay(5);
     
      //DisplayLoopTime();
      
      MyEspUsbHost.task(); 
      delay(5);
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
    WIFIManager.Loop();
    OTAHandler.Loop();
    
    if(OTAHandler.OTAOnly)
    {
        return;
    }

    if(WiFi.isConnected() && !WebSerialLogger.IsRunning())
    {
        WebSerialLogger.Begin(WebServer.GetServer());
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
      ch = WebSerialLogger.GetInput();
  }

  switch(ch)
  {
      case (char)-1:
          break;
      case 'i':
          MyEspUsbHost.DisplayInfo();
          break;
      case 'w':
          PS2Devices.MoveMouse(0, 10, 0);
          WebSerialLogger.println("w");
          break;
      case 's':
          PS2Devices.MoveMouse(0, -10, 0);
          WebSerialLogger.println("s");
          break;
      case 'a':  
          PS2Devices.MoveMouse(-10, 0, 0);
          WebSerialLogger.println("a");
          break;
      case 'd':  
          PS2Devices.MoveMouse(10, 0, 0);
          WebSerialLogger.println("d");
          break;
  }
}

void loop() 
{
    if(OTAHandler.OTAOnly)
    {
        //Serial.println("OTA only mode!");
        secondary_loop();
        return;
    }
}