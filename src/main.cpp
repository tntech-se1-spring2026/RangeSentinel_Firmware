#include <Arduino.h>
#include <array>
#include "shared_types.h"
#include "web_server.h"
#include "database_manager.h"

// sensor node
#ifdef NODE_TYPE_SENSOR

void setup() {

}

void loop() {

}

#endif

// ----------------------------------
// viewing node
#ifdef NODE_TYPE_VIEWER
#include <WiFi.h>
// #include <WebServer.h>
#include <LittleFS.h>

// standard HTTP port
// WebServer server(80);

unsigned long previousMillis = 0;
const long interval = 300000; // 5 minutes

void setup() {
    Serial.begin(115200);

    // start LittleFS. Exit if failed
    if (!LittleFS.begin(true)) {
        Serial.println("An error occurred while mounting LittleFS");
        return;
    }
    Serial.println("LittleFS mounted successfully");

    getDatabaseFromFS();

    // start access point
    // (SSID, Password)
    WiFi.softAP("Range-Sentinel-Gateway", "secure-sentinel-2026");
    Serial.print("Access IP Address: ");
    Serial.println(WiFi.softAPIP());  // should default to 192.168.4.1

    // setupWebServer(getDatabaseAsJson, getEventLogAsJson);
    startWebServer();
}

void loop() {
    // server.handleClient();

    // periodic saving
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        if (needsPersistence) {
            saveDatabaseToFS();
            previousMillis = currentMillis;
        }
    }
}

#endif
