#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

/* ************************IMPORTANT NOTE************************
* Anytime the networkDatabase object is read or written from, you
* must use the meshMutex to lock and unlock. Else you risk both
* cores attempting to simultaneously access it.
* 
* USAGE EXAMPLE:
* if (xSemaphoreTake(meshMutex, portMAX_DELAY)) { // LOCK
* 	// Put database access in place of this line
*	xSemaphoreGive(meshMutex); // UNLOCK
* }
*/

#include <array>
#include <LittleFS.h>
#include "shared_types.h"

#define MAX_NODES 10
// officially declared in main.cpp
extern std::array<NodeStatus, MAX_NODES> networkDatabase;

// global flag to track if we need to add to persistent memory
inline bool needsPersistence = false;

// Mutex prevents cores from looking at shared data simultaneously
SemaphoreHandle_t meshMutex;

// helper function to update database
inline void updateDatabase(NodeStatus incoming) {
    // LOCK
    if (xSemaphoreTake(meshMutex, portMAX_DELAY)) {
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
        // UNLOCK
        xSemaphoreGive(meshMutex);
    }
}

// converts entire active database to JSON array
inline String getDatabaseAsJson() {
    JsonDocument doc;
    JsonArray root = doc.to<JsonArray>();

    // LOCK
    if (xSemaphoreTake(meshMutex, portMAX_DELAY)) {
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
        // UNLOCK
        xSemaphoreGive(meshMutex);
    }

    String output;
    serializeJson(doc, output);
    return output;
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
            // LOCK
            if (xSemaphoreTake(meshMutex, portMAX_DELAY)) {
                networkDatabase[id].nodeId = id;
                networkDatabase[id].messageId = obj["mId"].as<uint32_t>();
                networkDatabase[id].batteryVoltage = obj["batt"].as<long>();
                networkDatabase[id].motionDetected = obj["motion"].as<bool>();
                networkDatabase[id].doorOpen = obj["door"].as<bool>();
                networkDatabase[id].nodeName = obj["name"].as<String>();
                // UNLOCK
                xSemaphoreGive(meshMutex);
            }
        }
    }
    Serial.println("DB: Database restored.");
}

#endif
