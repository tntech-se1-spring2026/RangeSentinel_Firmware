#include <Arduino.h>
#include <ArduinoJson.h>
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

#define MAX_NODES 10
NodeStatus networkDatabase[MAX_NODES] = {0};

// helper function to update database
void updateDatabase(NodeStatus incoming) {
    if (incoming.nodeId >= MAX_NODES) {
        return;
    }

    // only update if incoming is newer than what is already there
    if (incoming.messageId > networkDatabase[incoming.nodeId].messageId) {
        networkDatabase[incoming.nodeId] = incoming;
        Serial.printf("DB: Node %d updated (Msg %d)\n", incoming.nodeId, incoming.messageId);
    }
}

// converts entire active database to JSON array
String getDatabaseAsJson() {
    JsonDocument doc;
    JsonArray root = doc.to<JsonArray>();

    for (int i = 0; i < MAX_NODES, i++) {
        if (networkDatabase[i].messageId > 0) {
            JsonObject obj = root.add<JsonObject>();
            obj["id"] = networkDatabase[i].nodeId;
            obj["mId"] = networkDatabase[i].messageId;
            obj["batt"] = networkDatabase[i].batteryVoltage;
            obj["motion"] = networkDatabase[i].motionDetected;
            obj["door"] = networkDatabase[i].doorOpen;
        }
    }

    String output;
    serializeJson(doc, output);
    return output;
}

void setup() {

}

void loop() {

}

#endif
