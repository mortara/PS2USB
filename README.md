# USB HID to PS/2 adapter

This is an adapter to use USB Mouse and Keyboard on old computers with PS/2 connectors, based on an ESP32-S3.

This is currently very much WIP and a lot of things don't work as expected.

## Libraries

This project makes ue of the following libs:
 - https://github.com/hrko/esp32-ps2dev (I forked it to try some stuff)
 - https://github.com/mallardtheduck/EspUsbHost  (I forked it to try some stuff)
 - https://github.com/mathieucarbou/MycilaWebSerial
 - https://github.com/mathieucarbou/AsyncTCP
 - https://github.com/mathieucarbou/ESPAsyncWebServer

## Development

I was using PlatformIO initially but since i need more recent USB features of esp-idf, i switched to pioarduino:
https://github.com/pioarduino

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

I have routed the +5v lines of the PS/2 cables through a dip switch block so i can decide if the ESP or the USB device
will be powered by the PS/2 cables or not.

### Configuration

I've also attached 4 Pins of the ESP to a DIP Switch block in order to have config options:

ESP32 PIN    |   Option
-------------|----------------
38           |  Puts the device in OTA only mode
39           |  Swaps the PS/2 cables pins
40           |  Deactivates wifi, webserver, ota, webserial
41           |  currently unused

The Pins are set to HIGH with internal pullups and the dip switch connects them to GND when set to ON

## Mouse detection

When a USB mouse connects, the adapter reads its **HID report descriptor** — the machine-readable table the device provides that describes the exact binary layout of its input reports. The descriptor is parsed once at connect time to extract:

- **Buttons** — bit offset and count of button fields (usage page 0x09)
- **X / Y axes** — bit offset, bit width, and logical min/max (Generic Desktop usages 0x30 / 0x31)
- **Scroll wheel** — bit offset and bit width (usage 0x38), if present

This means the adapter works correctly with any mouse regardless of whether it uses 8-bit, 12-bit, 16-bit, or other coordinate widths, and regardless of how many padding or extra fields the report contains. The parsed layout is logged at connect time, for example:

```
[USB] Mouse layout: reportId=2 buttons=8@bit0 X=12bit@16 Y=12bit@28 Wheel=8bit@52
```

Each incoming report is then decoded by extracting signed integers at the exact bit positions the descriptor specified, without any assumptions about byte boundaries or field order.

If the descriptor cannot be parsed (e.g. the device does not expose one, or it contains no recognisable X/Y axes), the adapter falls back to the values the underlying EspUsbHost library provides, which handles standard boot-protocol mice correctly.

Coordinates are clamped to ±127 before being forwarded over PS/2. The PS/2 protocol carries overflow flags for larger movements, but many retro host drivers react to those flags with wild cursor jumps or a mouse reset; clamping avoids that while staying well below the 255 overflow threshold even when the host applies its optional 2:1 scale mode.

## Problems

Currently my main problem is the USB Host implementation on the ESP32:
 - It seems to have problems with USB hubs

