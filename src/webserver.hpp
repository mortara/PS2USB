#include "Webserver/webserver.hpp"


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

void handleReboot(AsyncWebServerRequest *request) {
    request->send(200, "text/html",
        "<html><body><p>Rebooting&hellip;</p>"
        "<script>setTimeout(()=>location.href='/',6000)</script>"
        "</body></html>");
    delay(200);
    ESP.restart();
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
        "<meta http-equiv='refresh' content='3'/>"
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

    // ── Activity ─────────────────────────────────────────────────────────────
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

    html += "</body></html>";
    request->send(200, "text/html", html);
}
