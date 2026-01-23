#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <Arduino.h>
#include <ArduinoJson.h>

// database schema to hold a node
struct NodeStatus {
    // could probably use some more memory-efficient data types if we gave that more thought
    uint8_t nodeId;  // 0-  255
    // to prevent duplicate alerts if both sensors register
    uint32_t messageId; // 0 - (2^32)-1...practically infinite
    float batteryVoltage;  // can calculate percentage left
    bool motionDetected;
    bool doorOpen;
};

// would need a funcion to translate struct to JSON


#endif
