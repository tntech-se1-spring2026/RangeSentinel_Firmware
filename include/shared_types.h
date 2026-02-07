#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <ArduinoJson.h>

// database schema to hold a node's status
// subject to change based on what we want
struct NodeStatus {
    uint32_t nodeId;
    uint32_t messageId;  // to prevent duplicate alerts if both sensors register
    float batteryVoltage;  // can calculate percentage left
    bool motionDetected;
    bool doorOpen;
    uint8_t nodeMACAddress[6];
    unsigned long lastSeen;
    char nodeName[64];
};

// pack a struct into an existing JsonObject
void nodeToJsonObject(const NodeStatus& status, JsonObject& obj);

// unpack a JsonObject into an existing struct
void jsonObjectToNode(const JsonObjectConst& obj, NodeStatus& status);

// translate C++ struct to JSON
// single node use
String nodeStatusToJson(const NodeStatus& status);

#endif