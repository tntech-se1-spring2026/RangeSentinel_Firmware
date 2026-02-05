#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <Arduino.h>

// database schema to hold a node's status
// subject to change based on what we want
struct NodeStatus {
    uint32_t nodeId;
    uint32_t messageId;  // to prevent duplicate alerts if both sensors register
    float batteryVoltage;  // can calculate percentage left
    bool motionDetected;
    bool doorOpen;
    char nodeName[20];
    unsigned long lastSeen;
};

// translate C++ struct to JSON
String nodeStatusToJson(const NodeStatus& status);

#endif
