#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <array>
#include <LittleFS.h>
#include "shared_types.h"

#define MAX_NODES 10
// officially declared in main.cpp
extern std::array<NodeStatus, MAX_NODES> networkDatabase;

// global flag to track if we need to add to persistent memory
inline bool needsPersistence = false;

// helper function to update database
inline void updateDatabase(NodeStatus incoming) {
    if (incoming.nodeId < networkDatabase.size()) {
        // only update if incoming is newer than what is already there
        if (incoming.messageId > networkDatabase.at(incoming.nodeId).messageId) {
            networkDatabase.at(incoming.nodeId) = incoming;
            needsPersistence = true;   // mark we want to save it eventually
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
// best used for web api
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

// saves database to LittleFS
inline bool saveDatabaseToFS() {
    if (!needsPersistence) {
        // nothing changed so skip
        return false;
    }

    // open file to write
    File file = LittleFS.open("/db_backup.json", "w");
    if (!file) {
        Serial.println("Failed to open DB file for writing.");
        return false;
    }

    JsonDocument doc;
    JsonArray root = doc.to<JsonArray>();

    for (const auto& node : networkDatabase) {
        if (node.nodeId != 0) {   // only save active nodes
            JsonObject obj = root.add<JsonObject>();
            obj["id"] = node.nodeId;
            obj["mId"] = node.messageId;
            obj["batt"] = node.batteryVoltage;
            obj["motion"] = node.motionDetected;
            obj["door"] = node.doorOpen;
            obj["name"] = node.nodeName;
        }
    }

    // serialize JSON to file
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to DB file.");
        file.close();
        return false;
    }

    file.close();
    needsPersistence = false;
    Serial.println("Database successfully backed to LittleFS.");
    return true;
}

// retrieves data from backup in LittleFS
inline void getDatabaseFromFS() {
    // check backup file exists
    if (!LittleFS.exists("/db_backup.json")) {
        Serial.println("DB: No backup file found, starting fresh.");
        return;
    }

    // ensure you can open it to read
    File file = LittleFS.open("/db_backup.json", "r");
    if (!file) {
        Serial.println("Failed to open file for reading.");
        return;
    }

    // deserialize json file
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("DB: Deserialization failed: ");
        Serial.println(error.c_str());
        return;
    }

    // convert to our C++ struct
    JsonArray array = doc.as<JsonArray>();
    for (JsonObject obj : array) {
        uint32_t id = obj["id"];
        if (id < MAX_NODES) {
            networkDatabase[id].nodeId = id;
            networkDatabase[id].messageId = obj["mId"];
            networkDatabase[id].batteryVoltage = obj["batt"];
            networkDatabase[id].motionDetected = obj["motion"];
            networkDatabase[id].doorOpen = obj["door"];
            strlcpy(networkDatabase[id].nodeName, obj["name"], sizeof(networkDatabase[id].nodeName));
        }
    }
    Serial.println("DB: Database restored.");
}

#endif
