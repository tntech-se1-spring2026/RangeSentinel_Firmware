#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <Arduino.h>
#include <ArduinoJson.h>

// database schema to hold a node's status
// subject to change based on what we want
struct NodeStatus {
    uint32_t nodeId;
    uint32_t messageId;  // to prevent duplicate alerts if both sensors register
    float batteryVoltage;  // can calculate percentage left
    bool motionDetected;
    bool doorOpen;
    char nodeName[20];
    bool isActive;
};

// translate C++ struct to JSON
inline String nodeStatusToJson(const NodeStatus& status) {
    JsonDocument doc;

    // map database entry (struct) to API fields in Json
    doc["id"] = status.nodeId;
    doc["mId"] = status.messageId;
    doc["batt"] = status.batteryVoltage;
    doc["motion"] = status.motionDetected;
    doc["door"] = status.doorOpen;

    String output;
    serializeJson(doc, output);

    return output;
}

#endif
