#include <ArduinoJson.h>
#include <LittleFS.h>
#include "database_manager.h"

#ifdef NODE_TYPE_VIEWER
#include <ESPAsyncWebServer.h>
extern AsyncWebSocket ws;
#endif

// live view
std::array<NodeRecord, MAX_NODES> networkDatabase = {};
// circular buffer history 
std::array<NodeRecord, MAX_LOG_ENTRIES> eventLog = {};
int logHead = 0;
bool needsPersistence = false;

// helper function to update database
void updateDatabase(MeshPacket incoming) {
    if (incoming.nodeId >= networkDatabase.size()) {
        Serial.printf("DB: Rejected node %d (out of bounds)\n", incoming.nodeId);
        return;
    }

    NodeRecord& currentRecord = networkDatabase.at(incoming.nodeId);

    // only update if incoming is newer than what is already there
    if (incoming.messageId > currentRecord.lastPacket.messageId) {
        currentRecord.lastPacket = incoming;
        currentRecord.lastSeen = millis();

        // assign default name if it doesn't have one
        if (strlen(currentRecord.nodeName) == 0) {
            snprintf(currentRecord.nodeName, sizeof(currentRecord.nodeName), "Node %d", incoming.nodeId);
        }

        // add to history
        eventLog[logHead] = currentRecord;
        logHead = (logHead + 1) % MAX_LOG_ENTRIES;

        needsPersistence = true;

        #ifdef NODE_TYPE_VIEWER
        JsonDocument updateDoc;
        JsonObject obj = updateDoc.to<JsonObject>();
        nodeRecordToJsonObject(currentRecord, obj);
        String output;
        serializeJson(updateDoc, output);
        ws.textAll(output);
        #endif

        Serial.printf("DB: Node %d updated & logged (Msg %d)\n", incoming.nodeId, incoming.messageId);
    }
    else {
        Serial.printf("DB: Ignored old/duplicate message %d from node %d\n", incoming.messageId, incoming.nodeId);
    }
}

// converts entire active database to JSON array
String getDatabaseAsJson() {
    JsonDocument doc;
    JsonArray root = doc.to<JsonArray>();

    for (const auto& record : networkDatabase) {
        if (record.lastPacket.messageId > 0) {
            JsonObject obj = root.add<JsonObject>();
            nodeRecordToJsonObject(record, obj);
        }
    }

    String output;
    serializeJson(doc, output);
    return output;
}

// converts circular event log to json
String getEventLogAsJson() {
    JsonDocument doc;
    JsonArray root = doc.to<JsonArray>();

    // work backward to get most recent activity first
    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        // handle wrapping around circular buffer and avoid negative indices
        int index = (logHead - 1 - i + MAX_LOG_ENTRIES) % MAX_LOG_ENTRIES;
        const NodeRecord& entry = eventLog[index];
        if (entry.lastPacket.nodeId > 0) {
            JsonObject obj = root.add<JsonObject>();
            nodeRecordToJsonObject(entry, obj);
        }
    }

    String output;
    serializeJson(doc, output);
    return output;
}

// saves database to LittleFS
bool saveDatabaseToFS() {
    if (!needsPersistence) {
        // nothing changed so skip
        return false;
    }

    // save current network status
    File file = LittleFS.open("/db_backup.json", "w");
    if (file) {
        JsonDocument doc;
        JsonArray root = doc.to<JsonArray>();

        for (const auto& record : networkDatabase) {
            if (record.lastPacket.nodeId > 0) {   // only save active nodes
                JsonObject obj = root.add<JsonObject>();
                nodeRecordToJsonObject(record, obj);
            }
        }
        if (serializeJson(doc, file) == 0) {
            Serial.println("DB: Failed to write current status to DB file.");
        }
        file.close();
    } 
    else {
        Serial.println("DB: Failed to open DB file for writing.");
        return false;
    }

    // save circular history log
    File logFile = LittleFS.open("/history_backup.json", "w");
    if (logFile) {
        JsonDocument logDoc;
        logDoc["head"] = logHead;  // save position in circular buffer
        JsonArray logs = logDoc["data"].to<JsonArray>();  // add label since we have head as well

        for (const auto& entry : eventLog) {
            if (entry.lastPacket.nodeId > 0) {
                JsonObject obj = logs.add<JsonObject>();
                nodeRecordToJsonObject(entry, obj);
            }
        }
        if (serializeJson(logDoc, logFile) == 0) {
            Serial.println("DB: Failed to write to log file.");
        }
        logFile.close();
    }
    else {
        Serial.println("DB: Failed to open log file for writing");
        return false;
    }

    needsPersistence = false;
    Serial.println("DB: Database and log file successfully backed to LittleFS.");
    return true;
}

// retrieves data from backup in LittleFS
void getDatabaseFromFS() {
    // restore current network status
    if (LittleFS.exists("/db_backup.json")) {
        File file = LittleFS.open("/db_backup.json", "r");
        if (file) {
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, file);
            if (!error) {
                JsonArray array = doc.as<JsonArray>();
                for (JsonObject obj : array) {
                    uint32_t id = obj["id"];
                    if (id < MAX_NODES) {
                        jsonObjectToNodeRecord(obj, networkDatabase[id]);
                    }
                }
                Serial.println("DB: Live status restored.");
            }
            else {
                Serial.println("DB: Deserialization error in db_backup.json");
                return;
            }
            file.close();
        }
    }

    // restore circular history log
    if (LittleFS.exists("/history_backup.json")) {
        File logFile = LittleFS.open("/history_backup.json", "r");
        if (logFile) {
            JsonDocument logDoc;
            DeserializationError error = deserializeJson(logDoc, logFile);
            if (!error) {
                logHead = logDoc["head"] | 0;   // so we know where to resume. defualt to 0
                JsonArray logs = logDoc["data"].as<JsonArray>();
                int i = 0;
                for (JsonObject obj : logs) {
                    if (i < MAX_LOG_ENTRIES) {
                        jsonObjectToNodeRecord(obj, eventLog[i]);
                        i++;
                    }
                }
                Serial.println("DB: History log restored.");
            }
            else {
                Serial.println("DB: Deserialization error in history_backup.json");
                return;
            }
            logFile.close();
        }
        else {
            Serial.println("DB: Failed to open file for reading.");
            return;
        }
    }
    else {
        Serial.println("DB: No backup file found, starting fresh.");
        return;
    }
}

// decides if incoming reading constitutes an alert
bool evaluateAlert(const SensorReading& r) {
    switch (r.type) {
    case SENSOR_TYPE_DOOR:
        return r.payload.asBool == true;
    case SENSOR_TYPE_MOTION:
        return r.payload.asBool == true;
    case SENSOR_TYPE_BATTERY:
        return r.payload.asFloat < 3.3;  // voltage less than 3.3 considered low battery
    default:
        return false;
    }
}

// development function to wipe db and logs to start fresh
// CAN'T THINK OF A USE CASE TO BE IN FINAL PRODUCT
void clearAllData() {
    LittleFS.remove("/db_backup.json");
    LittleFS.remove("/history_backup.json");
    networkDatabase.fill({});
    eventLog.fill({});
    logHead = 0;
    Serial.println("DB: Persistent storage wiped.");
}
