#include "main.h"

#include "EspUsbHost.h"
#include "ArduinoOTA.h"

// Lindy Kabel
// 1: Data -> gelb
// 2: Unused ->Orange
// 3: GND -> grün
// 4: VCC -> rot
// 5: Clock -> schwarz
// 6: Unused ->braun

bool ota_running = false;
unsigned long ota_timer = 0;

class MyEspUsbHost : public EspUsbHost {
  void onKeyboardKey(uint8_t ascii, uint8_t keycode, uint8_t modifier) {
    if (' ' <= ascii && ascii <= '~') {
      WebSerialLogger.printf("%c", ascii);
    } else if (ascii == '\r') {
      WebSerialLogger.println();
    }
  };

  void onMouseMoved(int16_t x, int16_t y, int8_t wheel) {
    WebSerialLogger.printf("Mouse moved: x=%d, y=%d, wheel=%d\n", x, y, wheel);
  };

  void onConfigured() {
    WebSerialLogger.println("USB device configured");
  };

  void onConnect() {
      WebSerialLogger.println("USB device connected");
  };
};

MyEspUsbHost usbHost;

void start_ota()
{
    ArduinoOTA.onStart([]() {

      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_FS
        type = "filesystem";
      }

      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      WebSerialLogger.println("Start updating " + type);
    });

    ArduinoOTA.onEnd([]() {
      WebSerialLogger.println("\nEnd");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      
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

    Serial.println("Initializing keyboard...");
    PS2Devices.InitKeyboard(5,4); 

    // initialize mouse
    Serial.println("Initializing mouse...");  
    PS2Devices.InitMouse(17,16);

    Serial.println("Initializing USB Host...");
    usbHost.begin();
    usbHost.setHIDLocal(HID_LOCAL_German);

    Serial.println("Starting wifi connection...");
    WIFIManager.Connect();

    Serial.println("Starting webserver...");
    WebServer.Setup();


}

void loop() {
  
  unsigned long now = millis();
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

    if(WiFi.isConnected() && !WebSerialLogger.IsRunning())
          WebSerialLogger.Begin(WebServer.GetServer());

    usbHost.task(); 

    char ch = -1;
    if (Serial.available()>0)
    {
        ch = Serial.read();

        switch(ch)
        {
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
            case '1':
                PS2Devices.PressMouse(esp32_ps2dev::PS2Mouse::Button::LEFT);
                WebSerialLogger.println("1");
                break;
            case '2':
                PS2Devices.PressMouse(esp32_ps2dev::PS2Mouse::Button::MIDDLE);
                WebSerialLogger.println("2");
                break;
            case '3':
                PS2Devices.PressMouse(esp32_ps2dev::PS2Mouse::Button::RIGHT);
                WebSerialLogger.println("3");
                break;
            case '4':
                PS2Devices.Type(esp32_ps2dev::scancodes::Key::K_F12);
                WebSerialLogger.println("F12");
                break;
            default:
                
                
                String str = String(ch);
                WebSerialLogger.println(str.c_str());
                PS2Devices.Type(str.c_str());
                break;
        }
    }

    WIFIManager.Loop();
    WebSerialLogger.Loop();
}