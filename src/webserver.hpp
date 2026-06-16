#include "Webserver/webserver.hpp"
#include "ps2devices.h"
#include "usbhost.h"
#include "main.h"


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

uint8_t mouseButtonIndex(esp32_ps2dev::PS2Mouse::Button button) {
    if (button == esp32_ps2dev::PS2Mouse::Button::LEFT) return 0;
    if (button == esp32_ps2dev::PS2Mouse::Button::RIGHT) return 1;
    if (button == esp32_ps2dev::PS2Mouse::Button::MIDDLE) return 2;
    return 0xFF;
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
        PS2Cmd cmd;
        cmd.type = PS2CmdType::TYPE_TEXT;
        strncpy(cmd.text, text.c_str(), sizeof(cmd.text) - 1);
        cmd.text[sizeof(cmd.text) - 1] = '\0';
        ps2Enqueue(cmd);
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

    PS2Cmd cmd;
    if (action == "down") {
        cmd.type = PS2CmdType::KEY_DOWN; cmd.key.key = key;
        ps2Enqueue(cmd);
    } else if (action == "up") {
        cmd.type = PS2CmdType::KEY_UP; cmd.key.key = key;
        ps2Enqueue(cmd);
    } else if (action == "tap") {
        cmd.type = PS2CmdType::KEY_DOWN; cmd.key.key = key;
        ps2Enqueue(cmd);
        cmd.type = PS2CmdType::KEY_UP;
        ps2Enqueue(cmd);
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
            PS2Cmd cmd;
            cmd.type = PS2CmdType::MOUSE_MOVE;
            cmd.move.x = dx; cmd.move.y = dy; cmd.move.wheel = wheel;
            ps2Enqueue(cmd);
            MyEspUsbHost.lastMouseMove.time = millis();
            MyEspUsbHost.lastMouseMove.x = dx;
            MyEspUsbHost.lastMouseMove.y = dy;
            MyEspUsbHost.lastMouseMove.wheel = wheel;
            MyEspUsbHost.lastMouseMove.ps2X = dx;
            MyEspUsbHost.lastMouseMove.ps2Y = dy;
            MyEspUsbHost.lastMouseMove.totalX += dx;
            MyEspUsbHost.lastMouseMove.totalY += dy;
            MyEspUsbHost.lastMouseMove.totalPs2X += dx;
            MyEspUsbHost.lastMouseMove.totalPs2Y += dy;
            MyEspUsbHost.lastMouseMove.totalWheel += wheel;
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
        PS2Cmd cmd;
        cmd.mouseBtn.button = button;
        if (action == "down") {
            cmd.type = PS2CmdType::MOUSE_PRESS;
            ps2Enqueue(cmd);
            MyEspUsbHost.RecordMouseButtonEvent(label, "pressed");
        } else if (action == "up") {
            cmd.type = PS2CmdType::MOUSE_RELEASE;
            ps2Enqueue(cmd);
            MyEspUsbHost.RecordMouseButtonEvent(label, "released");
            MyEspUsbHost.RecordMouseClick(mouseButtonIndex(button));
        } else if (action == "click") {
            cmd.type = PS2CmdType::MOUSE_CLICK;
            ps2Enqueue(cmd);
            MyEspUsbHost.RecordMouseButtonEvent(label, "click");
            MyEspUsbHost.RecordMouseClick(mouseButtonIndex(button));
        } else if (action == "doubleclick") {
            cmd.type = PS2CmdType::MOUSE_DOUBLE_CLICK;
            ps2Enqueue(cmd);
            MyEspUsbHost.RecordMouseButtonEvent(label, "double-click");
            MyEspUsbHost.RecordMouseDoubleClick(mouseButtonIndex(button), PS2Devices.GetMouseDoubleClickGapMillis());
        } else {
            request->send(400, "text/plain", "Unsupported mouse button action");
            return;
        }
        pmLogging.LogLn("[WEB] Mouse " + String(label) + " " + action);
        request->redirect("/");
        return;
    }

    request->send(400, "text/plain", "Unsupported mouse event");
}

void handleMouseSettings(AsyncWebServerRequest *request) {
    uint16_t clockHalf = (uint16_t)postInt(request, "clockHalf", PS2Devices.GetMouseClockHalfPeriodMicros(), 30, 80);
    uint16_t byteInterval = (uint16_t)postInt(request, "byteInterval", PS2Devices.GetMouseByteIntervalMicros(), 50, 2000);
    uint16_t speedPercent = (uint16_t)postInt(request, "speedPercent", MyEspUsbHost.GetMouseSpeedPercent(), 25, 200);

    MyEspUsbHost.mouseClockHalfMicros = clockHalf;
    MyEspUsbHost.mouseByteIntervalMicros = byteInterval;
    MyEspUsbHost.SetMouseSpeedPercent(speedPercent);
    bool saved = MyEspUsbHost.SaveMouseSettings();

    PS2Cmd cmd;
    cmd.type = PS2CmdType::SET_MOUSE_TIMING;
    cmd.mouseTiming.clockHalfMicros = clockHalf;
    cmd.mouseTiming.byteIntervalMicros = byteInterval;
    ps2Enqueue(cmd);

    pmLogging.LogLn("[WEB] Mouse settings speed=" + String(speedPercent) +
                    "% clockHalf=" + String(clockHalf) +
                    "us byteInterval=" + String(byteInterval) + "us");
    if (!saved) {
        request->send(500, "text/plain", "Mouse settings could not be saved");
        return;
    }
    request->redirect("/");
}

void handleMouseTiming(AsyncWebServerRequest *request) {
    handleMouseSettings(request);
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
        ":root{--bg:#eef1f4;--panel:#fff;--line:#ccd4dd;--text:#1d2733;--muted:#667487;--accent:#184c9f;--accent2:#0b6f57;--danger:#9b1c1c}"
        "*{box-sizing:border-box}"
        "body{background:var(--bg);font-family:Arial,Helvetica,Sans-Serif;color:var(--text);margin:0;padding:18px;line-height:1.35}"
        "body{max-width:1120px;margin-left:auto;margin-right:auto}"
        "h1{font-size:1.55rem;margin:0 0 6px;color:#10233f}"
        "h2{font-size:1.05rem;margin:22px 0 8px;color:#26384d;border-bottom:1px solid var(--line);padding-bottom:4px}"
        ".topbar{display:flex;gap:10px;align-items:center;flex-wrap:wrap;margin:0 0 14px;color:var(--muted)}"
        ".card{background:var(--panel);border:1px solid var(--line);border-radius:6px;padding:12px 14px;margin:8px 10px 8px 0;display:inline-block;min-width:280px;vertical-align:top;box-shadow:0 1px 2px rgba(20,30,45,.06)}"
        ".wide{min-width:min(100%,420px);max-width:760px}"
        ".controls{display:flex;gap:10px;flex-wrap:wrap;align-items:end;margin:10px 0}"
        "label{display:inline-block;font-size:.9rem;color:#344255;font-weight:bold}"
        "input,select,textarea{box-sizing:border-box;border:1px solid #aeb8c4;border-radius:5px;padding:6px 7px;font:inherit;background:#fff;color:var(--text)}"
        "textarea{width:100%;max-width:680px}"
        "input.num{width:86px}"
        "button{background:var(--accent);color:#fff;border:none;padding:7px 13px;border-radius:5px;cursor:pointer;font-size:1rem}"
        "button:hover{background:#123d81}"
        "button.reboot{background:var(--danger)}"
        "button.reboot:hover{background:#7f1717}"
        "table{border-collapse:collapse;margin:8px 0;background:#fff;box-shadow:0 1px 2px rgba(20,30,45,.04)}"
        "th,td{border:1px solid var(--line);padding:6px 10px;text-align:left;white-space:nowrap}"
        "th{background:#dbe6f5;color:#10233f;font-weight:bold}"
        ".none{color:var(--danger);font-weight:bold}"
        ".ok{color:var(--accent2);font-weight:bold}"
        ".hot{color:#b45a00;font-weight:bold}"
        ".dim{color:var(--muted);font-size:.9rem;font-weight:normal}"
        "a{color:var(--accent)}"
        "#rawlog{overflow-x:auto;padding-bottom:4px}"
        "@media(max-width:720px){body{padding:12px}.card{display:block;min-width:0;margin-right:0}.wide{min-width:0}table{font-size:.88rem}h1{font-size:1.3rem}}"
        "</style>"
        "</head><body>";

    html += "<h1>ESP32 PS/2 Adapter &mdash; " + String(WiFi.getHostname()) + "</h1>";
    html += "<div class='topbar'><span>Uptime: " + String(hr) + "h " + String(min % 60) + "m " + String(sec % 60) + "s</span>"
            "<a href='/config/'>Config</a>"
            "<form style='display:inline' method='POST' action='/reboot'"
            " onsubmit=\"return confirm('Reboot device?')\">"
            "<button class='reboot' type='submit'>Reboot</button></form></div>";

    // ── Device info ──────────────────────────────────────────────────────────
    html += "<h2>USB Device</h2>";
    if (!anyConnected) {
        html += "<div class='card'><span class='none'>No USB device connected</span></div>";
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
            "<option value='doubleclick'>Double Click</option>"
            "<option value='down'>Down</option>"
            "<option value='up'>Up</option>"
            "</select></label>"
            "<button type='submit'>Send Button</button>"
            "</form></div>";

    html += "<h2>Mouse Settings</h2>";
    html += "<div class='card wide'><b>PS/2 Mouse Calibration</b><br>";
    html += "<span class='dim'>Current speed: " + String(MyEspUsbHost.GetMouseSpeedPercent()) +
            "% &nbsp; clock half: " + String(PS2Devices.GetMouseClockHalfPeriodMicros()) +
            " us &nbsp; byte gap: " + String(PS2Devices.GetMouseByteIntervalMicros()) + " us</span><br>";
    html += "<form class='controls' method='POST' action='/ps2/mouse/settings'>"
            "<label>Speed %<br><input class='num' type='number' name='speedPercent' min='25' max='200' value='" +
            String(MyEspUsbHost.GetMouseSpeedPercent()) + "'></label>"
            "<label>Clock half us<br><input class='num' type='number' name='clockHalf' min='30' max='80' value='" +
            String(PS2Devices.GetMouseClockHalfPeriodMicros()) + "'></label>"
            "<label>Byte gap us<br><input class='num' type='number' name='byteInterval' min='50' max='2000' value='" +
            String(PS2Devices.GetMouseByteIntervalMicros()) + "'></label>"
            "<button type='submit'>Save</button>"
            "</form>"
            "</div>";

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
            uint8_t sampleRate = PS2Devices.GetMouseSampleRate();
            if (sampleRate) {
                html += "<span class='dim'>PS/2 sample rate: " + String(sampleRate) +
                        " Hz (" + String(PS2Devices.GetMouseReportIntervalMs()) + " ms)</span><br>";
            }
            html += "<span class='dim'>PS/2 click timing: down " +
                    String(PS2Devices.GetMouseClickPressMillis()) +
                    " ms, double gap " +
                    String(PS2Devices.GetMouseDoubleClickGapMillis()) + " ms</span><br>";
            html += "<span class='dim'>PS/2 resolution: " + String(1 << PS2Devices.GetMouseResolutionCode()) +
                    " count/mm";
            html += " &nbsp; scaling: " + String(PS2Devices.GetMouseScaleCode() ? "2:1" : "1:1");
            html += " &nbsp; mode: " + String(PS2Devices.GetMouseModeCode() == 0 ? "remote" :
                                             (PS2Devices.GetMouseModeCode() == 1 ? "stream" : "wrap"));
            html += " &nbsp; reporting: " + String(PS2Devices.IsMouseDataReportingEnabled() ? "on" : "off") +
                    "</span><br>";
            html += "<span class='dim'>Adapter speed: " + String(MyEspUsbHost.GetMouseSpeedPercent()) + "%</span><br>";

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
                html += "<span class='dim'>PS/2 sent: dx=" + String(MyEspUsbHost.lastMouseMove.ps2X) +
                        " dy=" + String(MyEspUsbHost.lastMouseMove.ps2Y) + "</span><br>";
                html += "<span class='dim'>Cumulative: dx=" + String(MyEspUsbHost.lastMouseMove.totalX) +
                        " dy=" + String(MyEspUsbHost.lastMouseMove.totalY);
                if (MyEspUsbHost.lastMouseMove.totalWheel != 0)
                    html += " wheel=" + String(MyEspUsbHost.lastMouseMove.totalWheel);
                html += "</span><br>";
                html += "<span class='dim'>PS/2 cumulative: dx=" + String(MyEspUsbHost.lastMouseMove.totalPs2X) +
                        " dy=" + String(MyEspUsbHost.lastMouseMove.totalPs2Y) + "</span><br>";
                html += "<span class='dim'>Total moves: " + String(MyEspUsbHost.lastMouseMove.totalMoves) + "</span><br>";
            }

            // Buttons
            if (MyEspUsbHost.lastMouseButton.time == 0) {
                html += "<span class='none'>No button event yet</span><br>";
            } else {
                html += "Button: <span class='ok'>" + String(MyEspUsbHost.lastMouseButton.description) + "</span>";
                html += " <span class='dim'>&mdash; " + elapsedStr(MyEspUsbHost.lastMouseButton.time) + "</span><br>";
                html += "<span class='dim'>Total clicks: " + String(MyEspUsbHost.lastMouseButton.totalClicks) + "</span><br>";
                if (MyEspUsbHost.lastMouseButton.lastClickIntervalMs != 0) {
                    html += "<span class='dim'>Last same-button click interval: " +
                            String(MyEspUsbHost.lastMouseButton.lastClickIntervalMs) +
                            " ms</span><br>";
                }
            }
            if (MyEspUsbHost.lastMouseButton.lastDoubleClickTime == 0) {
                html += "<span class='none'>No double-click yet</span>";
            } else {
                unsigned long doubleClickAge = millis() - MyEspUsbHost.lastMouseButton.lastDoubleClickTime;
                html += "Double-click: <span class='" + String(doubleClickAge < 3000 ? "hot" : "ok") + "'>" +
                        String(MyEspUsbHost.lastMouseButton.doubleClickDescription) + "</span>";
                html += " <span class='dim'>&mdash; " +
                        elapsedStr(MyEspUsbHost.lastMouseButton.lastDoubleClickTime) +
                        ", interval " +
                        String(MyEspUsbHost.lastMouseButton.lastClickIntervalMs) +
                        " ms, total " +
                        String(MyEspUsbHost.lastMouseButton.totalDoubleClicks) +
                        "</span>";
            }

            html += "</div>";
        }
    }

    // ── HID descriptor ───────────────────────────────────────────────────────
    html += "<h2>USB HID Descriptor</h2>";
    if (MyEspUsbHost.hidDesc.valid) {
        html += "<p style='margin:4px 0'><b>Interface:</b> " +
                String(MyEspUsbHost.hidDesc.interfaceNumber) +
                " &nbsp; <b>SubClass:</b> 0x" ;
        { char b[3]; snprintf(b,sizeof(b),"%02X",MyEspUsbHost.hidDesc.subClass); html+=b; }
        html += " &nbsp; <b>Protocol:</b> 0x";
        { char b[3]; snprintf(b,sizeof(b),"%02X",MyEspUsbHost.hidDesc.protocol); html+=b; }
        html += " &nbsp; <b>Len:</b> " + String(MyEspUsbHost.hidDesc.length) + "</p>";
        html += "<textarea readonly rows='4' style='width:100%;font-family:monospace;font-size:0.85em'>";
        for (uint16_t i = 0; i < MyEspUsbHost.hidDesc.length; i++) {
            if (i && (i % 16 == 0)) html += "\n";
            else if (i) html += " ";
            char b[3]; snprintf(b,sizeof(b),"%02X",MyEspUsbHost.hidDesc.data[i]); html+=b;
        }
        html += "</textarea>"
                "<p style='font-size:0.8em;color:#666'>Paste the hex bytes into "
                "<a href='https://eleccelerator.com/usbdescreqparser/' target='_blank'>"
                "eleccelerator.com/usbdescreqparser</a> to decode field layout.</p>";
    } else {
        html += "<p><i>No device connected (or descriptor not yet received).</i></p>";
    }

    // ── Raw USB mouse log ─────────────────────────────────────────────────────
    html += "<h2>USB Mouse Raw Data"
            " <label style='font-size:0.85em;font-weight:normal;margin-left:8px'>"
            "<input type='checkbox' id='rawChk' onchange=\""
            "localStorage.setItem('rawLog',this.checked);"
            "document.getElementById('rawlog').style.display=this.checked?'block':'none';"
            "\"> Show raw reports</label></h2>"
            "<div id='rawlog' style='display:none'>";

    // Build the table (newest entry first)
    html += "<table><tr><th>Age</th><th>rawLen</th><th>rawData</th>"
            "<th>rptLen</th><th>rptData</th>"
            "<th>lib&nbsp;x</th><th>lib&nbsp;y</th><th>lib&nbsp;w</th><th>lib&nbsp;btn</th><th>decoded&nbsp;btn</th></tr>";

    uint8_t head = MyEspUsbHost.mouseRawHead;
    for (uint8_t i = 0; i < MyEspUsbHostClass::MOUSE_RAW_LOG_SIZE; i++) {
        // Walk backwards from the slot before head (most recent first)
        uint8_t idx = (head + MyEspUsbHostClass::MOUSE_RAW_LOG_SIZE - 1 - i)
                      % MyEspUsbHostClass::MOUSE_RAW_LOG_SIZE;
        const auto &e = MyEspUsbHost.mouseRawLog[idx];
        if (e.time == 0) continue;

        html += "<tr><td>" + elapsedStr(e.time) + "</td>";
        html += "<td>" + String(e.rawLen) + "</td><td>";
        for (uint8_t b = 0; b < e.rawLen; b++) {
            if (b) html += " ";
            char buf[3]; snprintf(buf, sizeof(buf), "%02X", e.rawData[b]);
            html += buf;
        }
        html += "</td><td>" + String(e.rptLen) + "</td><td>";
        for (uint8_t b = 0; b < e.rptLen; b++) {
            if (b) html += " ";
            char buf[3]; snprintf(buf, sizeof(buf), "%02X", e.rptData[b]);
            html += buf;
        }
        html += "</td>";
        html += "<td>" + String(e.libX)  + "</td>";
        html += "<td>" + String(e.libY)  + "</td>";
        html += "<td>" + String(e.libWheel) + "</td>";
        char btnbuf[5]; snprintf(btnbuf, sizeof(btnbuf), "0x%02X", e.libButtons);
        html += "<td>" + String(btnbuf) + "</td>";
        char decodedBtnBuf[5]; snprintf(decodedBtnBuf, sizeof(decodedBtnBuf), "0x%02X", e.decodedButtons);
        html += "<td>" + String(decodedBtnBuf) + "</td></tr>";
    }
    html += "</table></div>";

    html += "<script>"
            "setTimeout(()=>{if(!document.querySelector('input:focus,textarea:focus,select:focus'))location.reload();},3000);"
            // Restore checkbox + visibility from localStorage on every load
            "(function(){"
            "var v=localStorage.getItem('rawLog')==='true';"
            "var cb=document.getElementById('rawChk');"
            "cb.checked=v;"
            "document.getElementById('rawlog').style.display=v?'block':'none';"
            "})();"
            "</script>";
    html += "</body></html>";
    request->send(200, "text/html", html);
}
