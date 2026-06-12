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
        } else if (action == "up") {
            cmd.type = PS2CmdType::MOUSE_RELEASE;
            ps2Enqueue(cmd);
        } else if (action == "click") {
            cmd.type = PS2CmdType::MOUSE_CLICK;
            ps2Enqueue(cmd);
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
            "<th>lib&nbsp;x</th><th>lib&nbsp;y</th><th>lib&nbsp;w</th><th>btn</th></tr>";

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
        html += "<td>" + String(btnbuf) + "</td></tr>";
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
