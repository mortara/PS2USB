#include "Webserver/webserver.hpp"
#include "ps2devices.h"
#include "usbhost.h"


String getDeviceTypeName(uint8_t cls, uint8_t subclass, uint8_t protocol) {
    if (cls == 0x03) {
        if (protocol == 0x01) return "HID Keyboard";
        if (protocol == 0x02) return "HID Mouse";
        if (subclass == 0x01) return "HID Boot Device";
        return "HID Device";
    }
    if (cls == 0x08) return "Mass Storage";
    if (cls == 0x02) return "Communications (CDC)";
    if (cls == 0x09) return "USB Hub";
    return "Class 0x" + String(cls, HEX);
}

String elapsedStr(unsigned long t) {
    if (t == 0) return "never";
    unsigned long sec = (millis() - t) / 1000;
    if (sec < 60)  return String(sec) + "s ago";
    if (sec < 3600) return String(sec / 60) + "m " + String(sec % 60) + "s ago";
    return String(sec / 3600) + "h ago";
}

String postValue(AsyncWebServerRequest *request, const char *name, const String& fallback = "") {
    if (!request->hasParam(name, true)) return fallback;
    return request->getParam(name, true)->value();
}

int postInt(AsyncWebServerRequest *request, const char *name, int fallback, int minValue, int maxValue) {
    int value = postValue(request, name, String(fallback)).toInt();
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

bool isWebKeyCodeSupported(uint8_t keycode) {
    if ((keycode >= 4 && keycode <= 49) ||
        (keycode >= 51 && keycode <= 57) ||
        (keycode >= 58 && keycode <= 99)) {
        return true;
    }
    return keycode == 0xE0 || keycode == 0xE1 || keycode == 0xE2 ||
           keycode == 0xE4 || keycode == 0xE5 || keycode == 0xE6;
}

bool parseMouseButton(const String& value, esp32_ps2dev::PS2Mouse::Button& button, const char *&label) {
    if (value == "left") {
        button = esp32_ps2dev::PS2Mouse::Button::LEFT;
        label = "Left";
        return true;
    }
    if (value == "right") {
        button = esp32_ps2dev::PS2Mouse::Button::RIGHT;
        label = "Right";
        return true;
    }
    if (value == "middle") {
        button = esp32_ps2dev::PS2Mouse::Button::MIDDLE;
        label = "Middle";
        return true;
    }
    return false;
}

void handleReboot(AsyncWebServerRequest *request) {
    request->send(200, "text/html",
        "<html><body><p>Rebooting&hellip;</p>"
        "<script>setTimeout(()=>location.href='/',6000)</script>"
        "</body></html>");
    delay(200);
    ESP.restart();
}

void handleKeyboardEvent(AsyncWebServerRequest *request) {
    String text = postValue(request, "text");
    if (text.length() > 0) {
        PS2Devices.Type(text.c_str());
        pmLogging.LogLn("[WEB] Keyboard text sent (" + String(text.length()) + " bytes)");
        request->redirect("/");
        return;
    }

    if (!request->hasParam("key", true)) {
        request->send(400, "text/plain", "Missing keyboard text or key");
        return;
    }

    int keyParam = postValue(request, "key").toInt();
    if (keyParam < 0 || keyParam > 255 || !isWebKeyCodeSupported((uint8_t)keyParam)) {
        request->send(400, "text/plain", "Unsupported key code");
        return;
    }

    uint8_t keycode = (uint8_t)keyParam;
    auto key = USBKeyCodeToPS2ScanCode(keycode);
    String action = postValue(request, "action", "tap");

    if (action == "down") {
        PS2Devices.KeyDown(key);
    } else if (action == "up") {
        PS2Devices.KeyUp(key);
    } else if (action == "tap") {
        PS2Devices.KeyDown(key);
        delay(20);
        PS2Devices.KeyUp(key);
    } else {
        request->send(400, "text/plain", "Unsupported keyboard action");
        return;
    }

    MyEspUsbHost.lastKey.time = millis();
    MyEspUsbHost.lastKey.keycode = keycode;
    MyEspUsbHost.lastKey.totalPresses++;
    pmLogging.LogLn("[WEB] Keyboard " + action + " keycode " + String(keycode));
    request->redirect("/");
}

void handleMouseEvent(AsyncWebServerRequest *request) {
    String event = postValue(request, "event", "move");

    if (event == "move") {
        int16_t dx = (int16_t)postInt(request, "dx", 0, -500, 500);
        int16_t dy = (int16_t)postInt(request, "dy", 0, -500, 500);
        int8_t wheel = (int8_t)postInt(request, "wheel", 0, -20, 20);

        if (dx != 0 || dy != 0 || wheel != 0) {
            PS2Devices.MoveMouse(dx, dy, wheel);
            MyEspUsbHost.lastMouseMove.time = millis();
            MyEspUsbHost.lastMouseMove.x = dx;
            MyEspUsbHost.lastMouseMove.y = dy;
            MyEspUsbHost.lastMouseMove.wheel = wheel;
            MyEspUsbHost.lastMouseMove.totalMoves++;
            pmLogging.LogLn("[WEB] Mouse move dx=" + String(dx) + " dy=" + String(dy) + " wheel=" + String(wheel));
        }

        request->redirect("/");
        return;
    }

    if (event == "button") {
        esp32_ps2dev::PS2Mouse::Button button;
        const char *label = "";
        if (!parseMouseButton(postValue(request, "button", "left"), button, label)) {
            request->send(400, "text/plain", "Unsupported mouse button");
            return;
        }

        String action = postValue(request, "action", "click");
        if (action == "down") {
            PS2Devices.PressMouseButton(button);
        } else if (action == "up") {
            PS2Devices.ReleaseMouseButton(button);
        } else if (action == "click") {
            PS2Devices.ClickMouseButton(button);
        } else {
            request->send(400, "text/plain", "Unsupported mouse button action");
            return;
        }

        snprintf(MyEspUsbHost.lastMouseButton.description,
                 sizeof(MyEspUsbHost.lastMouseButton.description),
                 "%s %s", label, action.c_str());
        MyEspUsbHost.lastMouseButton.time = millis();
        pmLogging.LogLn("[WEB] Mouse " + String(label) + " " + action);
        request->redirect("/");
        return;
    }

    request->send(400, "text/plain", "Unsupported mouse event");
}

void handleRoot(AsyncWebServerRequest *request) {

    int sec = millis() / 1000;
    int min = sec / 60;
    int hr = min / 60;

    // Query connected devices and interfaces
    EspUsbHostDeviceInfo devices[ESP_USB_HOST_MAX_DEVICES];
    size_t devCount = MyEspUsbHost.usb.getDevices(devices, ESP_USB_HOST_MAX_DEVICES);
    bool anyConnected = devCount > 0;
    bool anyHid = false;
    bool hasKeyboard = false;
    bool hasMouse = false;
    for (size_t d = 0; d < devCount; d++) {
        EspUsbHostInterfaceInfo intfs[ESP_USB_HOST_MAX_INTERFACES];
        size_t n = MyEspUsbHost.usb.getInterfaces(devices[d].address, intfs, ESP_USB_HOST_MAX_INTERFACES);
        for (size_t i = 0; i < n; i++) {
            if (intfs[i].interfaceClass == 0x03) {
                anyHid = true;
                if (intfs[i].interfaceProtocol == 0x01) hasKeyboard = true;
                if (intfs[i].interfaceProtocol == 0x02) hasMouse = true;
            }
        }
    }
    if (anyHid && !hasKeyboard && !hasMouse) { hasKeyboard = true; hasMouse = true; }

    String html =
        "<html><head>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'/>"
        "<title>ESP32 PS/2 Adapter</title>"
        "<style>"
        "body{background:#cccccc;font-family:Arial,Helvetica,Sans-Serif;color:#000088;margin:16px}"
        "h1{margin-bottom:4px}h2{margin:16px 0 6px}"
        "table{border-collapse:collapse;margin:4px 0}"
        "th,td{border:1px solid #000088;padding:4px 12px;text-align:left}"
        "th{background:#000088;color:#fff}"
        ".none{color:#880000}"
        ".ok{color:#006600;font-weight:bold}"
        ".dim{color:#666;font-size:0.9em}"
        ".card{background:#bbb;border:1px solid #999;border-radius:4px;padding:8px 14px;margin:6px 0;display:inline-block;min-width:260px;vertical-align:top}"
        ".wide{min-width:340px;max-width:720px}"
        ".controls{display:flex;gap:8px;flex-wrap:wrap;align-items:end;margin:8px 0}"
        "label{display:inline-block;font-size:0.9em;color:#333}"
        "input,select,textarea{box-sizing:border-box;border:1px solid #888;border-radius:3px;padding:4px;font:inherit}"
        "textarea{width:100%;max-width:680px}"
        "input.num{width:74px}"
        "button{background:#000088;color:#fff;border:none;padding:6px 12px;border-radius:4px;cursor:pointer;font-size:1em}"
        "button:hover{background:#1111aa}"
        "a{color:#000088}"
        "button.reboot{background:#880000;color:#fff;border:none;padding:6px 16px;border-radius:4px;cursor:pointer;font-size:1em}"
        "button.reboot:hover{background:#aa0000}"
        "</style>"
        "</head><body>";

    html += "<h1>ESP32 PS/2 Adapter &mdash; " + String(WiFi.getHostname()) + "</h1>";
    html += "<p>Uptime: " + String(hr) + "h " + String(min % 60) + "m " + String(sec % 60) + "s"
            " &nbsp;|&nbsp; <a href='/config/'>Config</a>"
            " &nbsp;|&nbsp; <form style='display:inline' method='POST' action='/reboot'"
            " onsubmit=\"return confirm('Reboot device?')\">"
            "<button class='reboot' type='submit'>Reboot</button></form></p>";

    // ── Device info ──────────────────────────────────────────────────────────
    html += "<h2>USB Device</h2>";
    if (!anyConnected) {
        html += "<p class='none'>No USB device connected</p>";
    } else {
        html += "<div class='card'>";

        // VID / PID / name
        if (MyEspUsbHost.cachedVid != 0) {
            char vidpid[24];
            snprintf(vidpid, sizeof(vidpid), "0x%04X / 0x%04X", MyEspUsbHost.cachedVid, MyEspUsbHost.cachedPid);
            html += "<b>" + String(vidpid) + "</b><br>";
        }
        if (MyEspUsbHost.cachedProduct[0] != '\0') {
            html += String(MyEspUsbHost.cachedProduct);
            if (MyEspUsbHost.cachedManufacturer[0] != '\0')
                html += " <span class='dim'>by " + String(MyEspUsbHost.cachedManufacturer) + "</span>";
            html += "<br>";
        }
        html += "</div>";

        // Interface table
        html += "<table><tr><th>Interface</th><th>Type</th><th>Class</th><th>Subclass</th><th>Protocol</th></tr>";
        for (size_t d = 0; d < devCount; d++) {
            EspUsbHostInterfaceInfo intfs[ESP_USB_HOST_MAX_INTERFACES];
            size_t n = MyEspUsbHost.usb.getInterfaces(devices[d].address, intfs, ESP_USB_HOST_MAX_INTERFACES);
            for (size_t i = 0; i < n; i++) {
                html += "<tr><td>" + String(intfs[i].number) + "</td>";
                html += "<td><b>" + getDeviceTypeName(intfs[i].interfaceClass, intfs[i].interfaceSubClass, intfs[i].interfaceProtocol) + "</b></td>";
                html += "<td>0x" + String(intfs[i].interfaceClass, HEX) + "</td>";
                html += "<td>0x" + String(intfs[i].interfaceSubClass, HEX) + "</td>";
                html += "<td>0x" + String(intfs[i].interfaceProtocol, HEX) + "</td></tr>";
            }
        }
        html += "</table>";
    }

    // PS/2 output controls
    html += "<h2>Send to PS/2 Ports</h2>";
    html += "<div class='card wide'><b>Keyboard</b><br>"
            "<form method='POST' action='/ps2/keyboard'>"
            "<label style='width:100%'>Text<br><textarea name='text' rows='3' maxlength='240' required></textarea></label><br>"
            "<button type='submit'>Type Text</button>"
            "</form>"
            "<form class='controls' method='POST' action='/ps2/keyboard'>"
            "<label>Key<br><select name='key'>"
            "<option value='40'>Enter</option>"
            "<option value='41'>Escape</option>"
            "<option value='42'>Backspace</option>"
            "<option value='43'>Tab</option>"
            "<option value='44'>Space</option>"
            "<option value='57'>Caps Lock</option>"
            "<option value='82'>Arrow Up</option>"
            "<option value='81'>Arrow Down</option>"
            "<option value='80'>Arrow Left</option>"
            "<option value='79'>Arrow Right</option>"
            "<option value='73'>Insert</option>"
            "<option value='76'>Delete</option>"
            "<option value='74'>Home</option>"
            "<option value='77'>End</option>"
            "<option value='75'>Page Up</option>"
            "<option value='78'>Page Down</option>"
            "<option value='58'>F1</option><option value='59'>F2</option>"
            "<option value='60'>F3</option><option value='61'>F4</option>"
            "<option value='62'>F5</option><option value='63'>F6</option>"
            "<option value='64'>F7</option><option value='65'>F8</option>"
            "<option value='66'>F9</option><option value='67'>F10</option>"
            "<option value='68'>F11</option><option value='69'>F12</option>"
            "<option value='224'>Left Ctrl</option>"
            "<option value='225'>Left Shift</option>"
            "<option value='226'>Left Alt</option>"
            "<option value='228'>Right Ctrl</option>"
            "<option value='229'>Right Shift</option>"
            "<option value='230'>Right Alt</option>"
            "</select></label>"
            "<label>Action<br><select name='action'>"
            "<option value='tap'>Tap</option>"
            "<option value='down'>Down</option>"
            "<option value='up'>Up</option>"
            "</select></label>"
            "<button type='submit'>Send Key</button>"
            "</form></div>";

    html += "<div class='card wide'><b>Mouse</b><br>"
            "<form class='controls' method='POST' action='/ps2/mouse'>"
            "<input type='hidden' name='event' value='move'>"
            "<label>X<br><input class='num' type='number' name='dx' min='-500' max='500' value='0'></label>"
            "<label>Y<br><input class='num' type='number' name='dy' min='-500' max='500' value='0'></label>"
            "<label>Wheel<br><input class='num' type='number' name='wheel' min='-20' max='20' value='0'></label>"
            "<button type='submit'>Move</button>"
            "</form>"
            "<div class='controls'>"
            "<form method='POST' action='/ps2/mouse'><input type='hidden' name='event' value='move'><input type='hidden' name='dx' value='0'><input type='hidden' name='dy' value='25'><input type='hidden' name='wheel' value='0'><button type='submit'>Up</button></form>"
            "<form method='POST' action='/ps2/mouse'><input type='hidden' name='event' value='move'><input type='hidden' name='dx' value='-25'><input type='hidden' name='dy' value='0'><input type='hidden' name='wheel' value='0'><button type='submit'>Left</button></form>"
            "<form method='POST' action='/ps2/mouse'><input type='hidden' name='event' value='move'><input type='hidden' name='dx' value='25'><input type='hidden' name='dy' value='0'><input type='hidden' name='wheel' value='0'><button type='submit'>Right</button></form>"
            "<form method='POST' action='/ps2/mouse'><input type='hidden' name='event' value='move'><input type='hidden' name='dx' value='0'><input type='hidden' name='dy' value='-25'><input type='hidden' name='wheel' value='0'><button type='submit'>Down</button></form>"
            "</div>"
            "<form class='controls' method='POST' action='/ps2/mouse'>"
            "<input type='hidden' name='event' value='button'>"
            "<label>Button<br><select name='button'>"
            "<option value='left'>Left</option>"
            "<option value='right'>Right</option>"
            "<option value='middle'>Middle</option>"
            "</select></label>"
            "<label>Action<br><select name='action'>"
            "<option value='click'>Click</option>"
            "<option value='down'>Down</option>"
            "<option value='up'>Up</option>"
            "</select></label>"
            "<button type='submit'>Send Button</button>"
            "</form></div>";

    // Activity
    if (anyConnected) {
        html += "<h2>Activity</h2>";

        if (hasKeyboard) {
            html += "<div class='card'><b>Keyboard</b><br>";
            if (MyEspUsbHost.lastKey.time == 0) {
                html += "<span class='none'>No key press yet</span>";
            } else {
                html += "Last key: <span class='ok'>keycode " + String(MyEspUsbHost.lastKey.keycode) + "</span>";
                html += " <span class='dim'>&mdash; " + elapsedStr(MyEspUsbHost.lastKey.time) + "</span><br>";
                html += "<span class='dim'>Total presses: " + String(MyEspUsbHost.lastKey.totalPresses) + "</span>";
            }
            html += "</div>";
        }

        if (hasMouse) {
            html += "<div class='card'><b>Mouse</b><br>";

            // Movement
            if (MyEspUsbHost.lastMouseMove.time == 0) {
                html += "<span class='none'>No movement yet</span><br>";
            } else {
                html += "Move: <span class='ok'>dx=" + String(MyEspUsbHost.lastMouseMove.x) +
                        " dy=" + String(MyEspUsbHost.lastMouseMove.y);
                if (MyEspUsbHost.lastMouseMove.wheel != 0)
                    html += " wheel=" + String(MyEspUsbHost.lastMouseMove.wheel);
                html += "</span>";
                html += " <span class='dim'>&mdash; " + elapsedStr(MyEspUsbHost.lastMouseMove.time) + "</span><br>";
                html += "<span class='dim'>Total moves: " + String(MyEspUsbHost.lastMouseMove.totalMoves) + "</span><br>";
            }

            // Buttons
            if (MyEspUsbHost.lastMouseButton.time == 0) {
                html += "<span class='none'>No button event yet</span>";
            } else {
                html += "Button: <span class='ok'>" + String(MyEspUsbHost.lastMouseButton.description) + "</span>";
                html += " <span class='dim'>&mdash; " + elapsedStr(MyEspUsbHost.lastMouseButton.time) + "</span>";
            }

            html += "</div>";
        }
    }

    html += "<script>setTimeout(()=>{if(!document.querySelector('input:focus,textarea:focus,select:focus'))location.reload();},3000);</script>";
    html += "</body></html>";
    request->send(200, "text/html", html);
}
