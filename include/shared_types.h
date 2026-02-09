#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <Arduino.h>
#include <ArduinoJson.h>

#define MAX_SENSORS_PER_PACKET 4
#define MAX_PACKET_SIZE RMT_MEM_64

// what kind of data?
typedef enum {
    SENSOR_TYPE_NONE = 0x00,
    SENSOR_TYPE_DOOR = 0x01,
    SENSOR_TYPE_MOTION = 0x02,
    SENSOR_TYPE_BATTERY = 0x03,
    SENSOR_TYPE_ERROR = 0xFF
} SensorType;

// payload holder
typedef union {
    bool asBool;
    float asFloat;
    uint8_t asByte;
} SensorData;

// single sensor event
struct SensorReading {
    uint8_t sensorIndex;  // which sensor
    SensorType type;  // format identifier
    SensorData payload;  // actual data
};

// what is sent over LoRa
struct MeshPacket {
    uint8_t nodeId;
    uint32_t messageId;
    uint8_t readingCount;
    SensorReading readings[MAX_SENSORS_PER_PACKET];
};

// metadata not sent over radio (will be stored in viewing node for comparison)
struct NodeRecord {
    MeshPacket lastPacket;
    char nodeName[20];
    unsigned long lastSeen;  // local timestamp
};



// // database schema to hold a node's status
// // subject to change based on what we want
// struct NodeStatus {
//     uint32_t nodeId;
//     uint32_t messageId;  // to prevent duplicate alerts if both sensors register
//     float batteryVoltage;  // can calculate percentage left
//     bool motionDetected;
//     bool doorOpen;
//     char nodeName[20];
//     unsigned long lastSeen;
// };

// // pack a struct into an existing JsonObject
// void nodeToJsonObject(const NodeStatus& status, JsonObject& obj);

// // unpack a JsonObject into an existing struct
// void jsonObjectToNode(const JsonObjectConst& obj, NodeStatus& status);

// // translate C++ struct to JSON
// // single node use
// String nodeStatusToJson(const NodeStatus& status);

#endif
