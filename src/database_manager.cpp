#include "database_manager.h"

#ifdef NODE_TYPE_VIEWER
#include <ESPAsyncWebServer.h>
extern AsyncWebSocket ws;
#endif

std::array<NodeRecord, MAX_NODES> networkDatabase   = {}; // live view
std::array<NodeRecord, MAX_LOG_ENTRIES> eventLog    = {}; // circular buffer history 
int logHead                                         = 0;
bool needsPersistence                               = false;
size_t numNodesInNetwork                            = 0;
SemaphoreHandle_t meshMutex                         = NULL;

bool appendToNetwork(NodeRecord newStatus){
    if(numNodesInNetwork < MAX_NODES){
        networkDatabase[numNodesInNetwork] = newStatus;
        numNodesInNetwork++;
        return true;
    }else{
        return false;
    }
}

// TODO: Finish this function
void updateDatabase(MeshPacket incoming, uint8_t nodeID){
    NodeRecord& currentRecord = networkDatabase.at(nodeID - 1);

    // only update if incoming is newer than what is already there
    if (incoming.messageId > currentRecord.lastPacket.messageId) {
        bool foundAlert = false;

        // look for alerts in the readings
        for (int i = 0; i < incoming.readingCount; i++) {
            // save alert status to individual reading
            incoming.readings[i].isAlert = evaluateAlert(incoming.readings[i]);
            if (evaluateAlert(incoming.readings[i])) {
                foundAlert = true;
            }
        }

        currentRecord.lastPacket = incoming;
        currentRecord.hasActiveAlert = foundAlert;  // store alert status
        currentRecord.lastSeen = millis();

        // if there is an alert, lock the latch (user would have to clear / acknowledge alert to reset it)
        if (foundAlert) {
            currentRecord.alertLatched = true;
        }

        // assign default name if it doesn't have one
        if (strlen(currentRecord.nodeName) == 0) {
            snprintf(currentRecord.nodeName, sizeof(currentRecord.nodeName), "Node %d", nodeID);
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

        // blankspace to keep logs aligned
        Serial.printf("%s DB: Node %d updated & logged (Msg %d)\n", foundAlert ? "[ALERT!]" : "        ", nodeID, incoming.messageId);
    }
    else {
        Serial.printf("DB: Ignored old/duplicate message %d from node %d\n", incoming.messageId, nodeID);
    }
}

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

String getEventLogAsJson() {
    JsonDocument doc;
    JsonArray root = doc.to<JsonArray>();

    // work backward to get most recent activity first
    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        // handle wrapping around circular buffer and avoid negative indices
        int index = (logHead - 1 - i + MAX_LOG_ENTRIES) % MAX_LOG_ENTRIES;
        const NodeRecord& entry = eventLog[index];
        if (entry.nodeID > 0) {
            JsonObject obj = root.add<JsonObject>();
            nodeRecordToJsonObject(entry, obj);
        }
    }

    String output;
    serializeJson(doc, output);
    return output;
}

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
            if (record.nodeID > 0) {   // only save active nodes
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
            if (entry.nodeID > 0) {
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
                logHead = logDoc["head"] | 0;   // so we know where to resume. default to 0
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
bool evaluateAlert(const Reading& r) {
    switch (r.type) {
    case DOOR_SENSOR:
        return r.payload.asBool == true;
    case MOTION_SENSOR:
        return r.payload.asBool == true;
    case BATTERY_SENSOR:
        return r.payload.asFloat < BATTERY_THRESHOLD;
    default:
        return false;
    }
}

// manually clear the latch for a specific node
bool clearAlertLatch(uint8_t nodeId) {
    if (nodeId >= MAX_NODES) {
        return false;  // invalid ID
    }

    // reset latch
    networkDatabase.at(nodeId).alertLatched = false;

    // trigger backup to LittleFS
    needsPersistence = true;
    saveDatabaseToFS();

    Serial.printf("DB: Alert latch cleared for node %d\n", nodeId);
    return true;  // success

}


void clearAllData() {
    LittleFS.remove("/db_backup.json");
    LittleFS.remove("/history_backup.json");
    networkDatabase.fill({});
    eventLog.fill({});
    logHead = 0;
    Serial.println("DB: Persistent storage wiped.");
}

int findNodeIndexByMAC(uint8_t* mac) {
    for (int i = 0; i < networkDatabase.size(); i++) {
        if (memcmp(networkDatabase[i].MACAddress, mac, 6) == 0) {
            return i;
        }
    }
    return -1;
}

uint8_t addNodeToNetworkDatabase(MeshPacket firstTransmission){
    NodeRecord newNode;
    newNode.nodeID = numNodesInNetwork++;
    newNode.lastPacket = firstTransmission;
    newNode.lastSeen = millis();
    memcpy(newNode.MACAddress, getReadingOfType(firstTransmission.readings, REQUEST_TO_ASSIGN)->payload.asMAC, 6);
    // assign default name
    snprintf(newNode.nodeName, sizeof(newNode.nodeName), "Node %d", newNode.nodeID);

    (void)appendToNetwork(newNode);

    // add to history
    eventLog[logHead] = newNode;
    logHead = (logHead + 1) % MAX_LOG_ENTRIES;

    return newNode.nodeID;
}