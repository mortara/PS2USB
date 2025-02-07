# USB HID to PS/2 adapter

This is an adapter to use USB Mouse and Keyboard on old computers with PS/2 connectors, based on an ESP32-S3.

This is currently very much WIP and a lot of things don't work as expected.

## Libraries

This project makes ue of the following libs:
 - https://github.com/hrko/esp32-ps2dev
 - https://github.com/mallardtheduck/EspUsbHost  (I forked it to try some stuff)
 - https://github.com/mathieucarbou/MycilaWebSerial
 - https://github.com/mathieucarbou/AsyncTCP
 - https://github.com/mathieucarbou/ESPAsyncWebServer

## Hardware

I am using a ESP32-S3-WROOM-1. 

### PS/2 cables 

The PS/2 cables are wired like this:

PS/2          |   ESP32-S3 PIN
--------------|----------------
Mouse CLK     |   17
Mouse DATA    |   16
Mouse GND     |   GND
Mouse +5v     |   5v 
Keyboard CLK  |   5
Keyboard DATA |   4
Keyboard GND  |   GND
Keyboard +5v  |   VBUS of USB socket

### USB Socket
There is a USB Type-A socket connected like this:

USB          |   ESP32-S3 PIN
-------------|-----------------
D+           |   21
D-           |   20
GND          |   GND
VBus         |   PS/2 Keyboard 5v

### Configuration

I've also attached 4 Pins of the ESP to a DIP Switch block in order to have config options:

ESP32 PIN    |   Option
-------------|----------------
39           |  Puts the device in OTA only mode
40           |  Swaps the PS/2 cables pins
41           |  Deactivates wifi, webserver, ota, webserial
42           |  currently unused

The Pins are set to HIGH with internal pullups and the dip switch connects them to GND when set to ON


