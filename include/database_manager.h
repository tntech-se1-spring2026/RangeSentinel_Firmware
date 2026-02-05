#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <array>
#include <LittleFS.h>
#include "shared_types.h"

#define MAX_NODES 10
#define MAX_LOG_ENTRIES 100

// officially declared in main.cpp
// live view
extern std::array<NodeStatus, MAX_NODES> networkDatabase;
// circular history
std::array<NodeStatus, MAX_LOG_ENTRIES> eventLog;
int logHead = 0;

// global flag to track if we need to add to persistent memory
inline bool needsPersistence = false;

// helper function to update database
inline void updateDatabase(NodeStatus incoming) {
    incoming.lastSeen = millis();   // update timestamp

    if (incoming.nodeId < networkDatabase.size()) {
        // only update if incoming is newer than what is already there
        if (incoming.messageId > networkDatabase.at(incoming.nodeId).messageId) {
            // update live view
            networkDatabase.at(incoming.nodeId) = incoming;

            // add to log
            eventLog[logHead] = incoming;
            logHead = (logHead + 1) % MAX_LOG_ENTRIES;   // stop overflow and make it 

            needsPersistence = true;   // mark we want to save it persistently when timer reaches the 5 min mark
            Serial.printf("DB: Node %d updated & logged (Msg %d)\n", incoming.nodeId, incoming.messageId);
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
            obj["ts"] = node.lastSeen;
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
            obj["ls"] = node.lastSeen;
        }
    }

    // serialize JSON to file
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to DB file.");
        file.close();
        return false;
    }

    File logFile = LittleFS.open("/history_backup.json", "w");
    if (!logFile) {
        Serial.println("Failed to open history file for writing");
        return false;
    }

    JsonDocument logDoc;
    logDoc["head"] = logHead;   // save position in circular buffer
    JsonArray logs = logDoc.add<JsonArray>();
    for (const auto& entry : eventLog) {
        if (entry.nodeId > 0) {
            JsonObject obj = logs.add<JsonObject>();
            obj["id"] = entry.nodeId;
            obj["mId"] = entry.messageId;
            obj["batt"] = entry.batteryVoltage;
            obj["motion"] = entry.motionDetected;
            obj["door"] = entry.doorOpen;
            obj["name"] = entry.nodeName;
            obj["ls"] = entry.lastSeen;
        }
    }

    // serialize JSON to file
    if (serializeJson(logDoc, logFile) == 0) {
        Serial.println("Failed to write to log file.");
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
            networkDatabase[id].lastSeen = obj["ls"];
        }
    }
    Serial.println("DB: Database restored.");
}

// converts circular event log to json
inline String getEventLogAsJson() {
    JsonDocument doc;
    JsonArray root = doc.to<JsonArray>();

    // work backward to get most recent activity first
    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        // handle wrapping around circular buffer and avoid negative indices
        int index = (logHead - 1 - i + MAX_LOG_ENTRIES) % MAX_LOG_ENTRIES;
        const NodeStatus& entry = eventLog[index];
        if (entry.nodeId > 0) {
            JsonObject obj = root.add<JsonObject>();
            obj["id"] = entry.nodeId;
            obj["mId"] = entry.messageId;
            obj["batt"] = entry.batteryVoltage;
            obj["motion"] = entry.motionDetected;
            obj["door"] = entry.doorOpen;
            obj["name"] = String(entry.nodeName);
            obj["ts"] = entry.lastSeen;
        }
    }

    String output;
    serializeJson(doc, output);
    return output;
}

#endif
