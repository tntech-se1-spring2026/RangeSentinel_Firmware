#include "web_server.h"
#include <LittleFS.h>

void setupWebServer(String (*getStatusJson)()) {
    // home page
    server.on("/", HTTP_GET, []() {
        File file = LittleFS.open("/index.html", "r");
        if (!file) {
            server.send(404, "text/plain", "Web dashboard file not found in LittleFS");
            return;
        }
        server.streamFile(file, "text/html");
    });

    // JSON data api
    server.on("/api/status", HTTP_GET, [getStatusJson]() {
        server.send(200, "application/json", getStatusJson());
    });
    server.begin();
    Serial.println("HTTP server started");
}
