#include "wifimanager.hpp"

void WIFIManagerClass::StartUp()
{
    WiFi.disconnect();

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(true);
    WiFi.setAutoReconnect(true);
    //Connect();

    WiFi.setHostname("ESP32Radio");
}

bool WIFIManagerClass::Connect()
{
    if(WiFi.status() == WL_CONNECTED || connecting)
    {
        //WebSerialLogger.println("Already connected or connecting");
        return true;
    }

    connecting = true;
    WiFi.disconnect();
    WiFi.begin(_credentials.SSID.c_str(), _credentials.PASS.c_str());
    WebSerialLogger.println("Connecting to WiFi ..");
    _lastConnectionTry = millis();
    wifi_timer = millis();
    /*int timeout = 10;
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(1000);
        timeout--;
        if(timeout == 0)
        {
            Serial.println("WIFI timeout ......");
            return false;
        }
    }
    connected = true;
    DisplayInfo();*/
    return true;
}

void WIFIManagerClass::Disconnect()
{
    WebSerialLogger.println("disonnecting from WiFi ..");
    WiFi.disconnect();
    
    connecting = false;
    connected = false;
}

void WIFIManagerClass::DisplayInfo(){
     
    WebSerialLogger.print("[*] Network information for ");
    WebSerialLogger.println(_credentials.SSID);

    WebSerialLogger.println("[+] BSSID : " + WiFi.BSSIDstr());
    WebSerialLogger.print("[+] Gateway IP : ");
    WebSerialLogger.println(WiFi.gatewayIP().toString());
    WebSerialLogger.print("[+] DNS IP : ");
    WebSerialLogger.println(WiFi.dnsIP().toString());   
    WebSerialLogger.println((String)"[+] RSSI : " + String(WiFi.RSSI()) + " dB");
    WebSerialLogger.print("[+] ESP32 IP : ");
    WebSerialLogger.println(WiFi.localIP().toString());
    WebSerialLogger.print("[+] Subnet Mask : ");
    WebSerialLogger.println(WiFi.subnetMask().toString());
    
}

bool WIFIManagerClass::IsConnected()
{
    return connected;
}

unsigned long WIFIManagerClass::LastConnectionTry()
{
    return _lastConnectionTry;
}

void WIFIManagerClass::Loop()
{
    unsigned long currentMillis = millis();

    if(currentMillis - wifi_timer > 100UL)
    {
        wifi_timer = currentMillis;
        connected = WiFi.isConnected();

        if(connecting && connected)
        {
            WebSerialLogger.println("WiFi connected!");
            DisplayInfo();
            connecting = false;
        }

        // if WiFi is down, try reconnecting
        if (!connecting && !connected && (currentMillis - _lastConnectionTry >= interval)) 
        {
            WebSerialLogger.println("Reconnecting to WiFi...");
            WiFi.reconnect();
            connecting = true;
            _lastConnectionTry = currentMillis;
        }
    }
}

WIFIManagerClass WIFIManager;