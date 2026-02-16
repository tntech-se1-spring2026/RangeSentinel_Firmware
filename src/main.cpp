#include <Arduino.h>
#include <array>
#include "shared_types.h"
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
#include "web_server.h"
#include <LittleFS.h>
#include <DNSServer.h>

// standard HTTP port
#define HTTP_PORT 80
static AsyncWebServer server(HTTP_PORT);

DNSServer dnsServer;
#define DNS_PORT 53

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
    WiFi.softAP("Range-Sentinel-Gateway", "password");
    Serial.print("Access IP Address: ");
    Serial.println(WiFi.softAPIP());  // should default to 192.168.4.1

    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    startWebServer(&server);

}

void loop() {
    dnsServer.processNextRequest();
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
