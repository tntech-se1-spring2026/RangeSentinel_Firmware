#include <Arduino.h>
#include <ArduinoJson.h>
#include <array>
#include "shared_types.h"

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
#include <WebServer.h>
#include <LittleFS.h>

#define MAX_NODES 10

// standard HTTP port
WebServer server(80);
std::array<NodeStatus, MAX_NODES> networkDatabase = {};

// helper function to update database
void updateDatabase(NodeStatus incoming) {
    if (incoming.nodeId < networkDatabase.size()) {
        // only update if incoming is newer than what is already there
        if (incoming.messageId > networkDatabase.at(incoming.nodeId).messageId) {
            networkDatabase.at(incoming.nodeId) = incoming;
            Serial.printf("DB: Node %d updated (Msg %d)\n", incoming.nodeId, incoming.messageId);
        } 
        else {
            Serial.printf("DB: Ignored old msg %d from node %d\n", incoming.messageId, incoming.nodeId);
        }
    }
    else {
        Serial.printf("DB: Rejected node %d (out of bounds)\n", incoming.nodeId);
    }
}

// converts entire active database to JSON array
String getDatabaseAsJson() {
    JsonDocument doc;
    JsonArray root = doc.to<JsonArray>();

    for (const auto& node : networkDatabase) {
        if (node.messageId > 0) {
            JsonObject obj = root.add<JsonObject>();
            obj["id"] = node.nodeId;
            obj["mId"] = node.messageId;
            obj["batt"] = node.batteryVoltage;
            obj["motion"] = node.motionDetected;
            obj["door"] = node.doorOpen;
            obj["name"] = String(node.nodeName);
        }
    }

    String output;
    serializeJson(doc, output);
    return output;
}

void setup() {
    Serial.begin(115200);

    // start LittleFS. Exit if failed
    if (!LittleFS.begin(true)) {
        Serial.println("An error occurred while mounting LittleFS");
        return;
    }
    Serial.println("LittleFS mounted successfully");

    // start access point
    // (SSID, Password)
    WiFi.softAP("Range-Sentinel-Gateway", "secure-sentinel-2026");
    Serial.print("Gateway IP Address: ");
    Serial.println(WiFi.softAPIP());  // should default to 192.168.4.1

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
    server.on("/api/status", HTTP_GET, []() {
        server.send(200, "application/json", getDatabaseAsJson());
    });
    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}

#endif
