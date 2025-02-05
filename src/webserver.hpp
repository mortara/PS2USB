#include <Arduino.h>
#include <DNSServer.h>
#include "WiFi.h"
#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"


#ifndef WEBSERVER_H
#define WEBSERVER_H

class WebServerClass
{
    private:
        AsyncWebServer *_webserver = NULL;
        DNSServer dnsServer;
     
    public:
        void Setup();
        AsyncWebServer *GetServer();

} ;

extern WebServerClass WebServer;

#endif