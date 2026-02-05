#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <array>
#include "shared_types.h"

#define MAX_NODES 10
// officially declared in main.cpp
extern std::array<NodeStatus, MAX_NODES> networkDatabase;

// helper function to update database
inline void updateDatabase(NodeStatus incoming) {
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
inline String getDatabaseAsJson() {
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

#endif
