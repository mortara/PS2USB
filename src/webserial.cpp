#include "webserial.hpp"

String inputString;

void recvMsg(const uint8_t *data, size_t len)
{
    Serial.println("Received Data...");
    String d = "";
    for(int i=0; i < len; i++){
        d += char(data[i]);
    }

    inputString = inputString + d;

    Serial.println(d);
    
}

void WebSerialLoggerClass::Start()
{
    Serial.println("Starting webserial server!");
    
}

void WebSerialLoggerClass::Begin(AsyncWebServer *_server)
{
    Serial.println("Starting webserial connection!");

    webSerial.begin(_server);
    webSerial.onMessage(recvMsg);

    webSerial.println("Hello from ESP32!");
    
    running = true;
}

bool WebSerialLoggerClass::IsRunning()
{
    return running;
    
}

char WebSerialLoggerClass::GetInput()
{
    if(inputString == "")
        return -1;

    char c = inputString[0];
    inputString = inputString.substring(1);
    return c;
}

void WebSerialLoggerClass::printf(const char *text, ...)
{
    Serial.printf(text);

    if(running)
        webSerial.print(text);
}

void WebSerialLoggerClass::print(const char *text)
{
    Serial.print(text);

    if(running)
        webSerial.print(text);
}

void WebSerialLoggerClass::print(String text)
{
    Serial.print(text);

    if(running)
        webSerial.print(text);
}

void WebSerialLoggerClass::println(const char *text)
{
    Serial.println(text);

    if(running)
        webSerial.println(text);
}

void WebSerialLoggerClass::println(String text)
{
    Serial.println(text);

    if(running)
        webSerial.println(text);

}

void WebSerialLoggerClass::Flush()
{
    if(running)
        webSerial.flush();
}

WebSerialLoggerClass WebSerialLogger;