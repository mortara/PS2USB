#include "main.h"

bool setup_done = false;
bool ota_only_mode = false;
bool ota_running = false;
unsigned long ota_timer = 0;

const int SWITCH_PINS[] = { 38, 27, 14, 12 };

void start_ota()
{
    ArduinoOTA.onStart([]() 
    {

      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_FS
        type = "filesystem";
      }

      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      WebSerialLogger.println("Start updating " + type);
      ota_only_mode = true;
    });

    ArduinoOTA.onEnd([]() {
      WebSerialLogger.println("\nEnd");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
    {
      
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
    
    ArduinoOTA.begin();
    Serial.println("OTA started");
    ota_running = true;
    ota_timer = millis();
}

void setup() {

    Serial.begin(115200);
    Serial.println("Hello, world!");

    pinMode(SWITCH_PINS[0], INPUT_PULLUP);

    int state = digitalRead(SWITCH_PINS[0]);

    if (state == ON)
    {
        ota_only_mode = true;
        WIFIManager.Connect();
    }
    else
    {
      Serial.println("Initializing keyboard...");
      PS2Devices.InitKeyboard(KEYBOARD_CLK,KEYBOARD_DATA); 

      // initialize mouse
      Serial.println("Initializing mouse...");  
      PS2Devices.InitMouse(MOUSE_CLK,MOUSE_DATA);

      Serial.println("Initializing USB Host...");
      MyEspUsbHost.init();
      
      Serial.println("Starting wifi connection...");
      WIFIManager.Connect();

      Serial.println("Starting webserver...");
      WebServer.Setup();
    }

    setup_done = true;
}

void loop() {
  
  if(!setup_done)
  {
    return;
  }

  unsigned long now = millis();
    WIFIManager.Loop();

    if(now - ota_timer > 100UL)
    {
     
      if(ota_running)
        ArduinoOTA.handle();
      else
      {
        if(WiFi.isConnected())
          start_ota();
      }
      ota_timer = now;
    }

    if(ota_only_mode)
    {
        return;
    }

    if(WiFi.isConnected() && !WebSerialLogger.IsRunning())
    {
          WebSerialLogger.Begin(WebServer.GetServer());
    }
    
    MyEspUsbHost.task(); 

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

    WebSerialLogger.Loop();
}