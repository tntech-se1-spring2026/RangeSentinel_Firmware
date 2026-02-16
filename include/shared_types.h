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
    bool isAlert;  // flag to indicate if this reading is a security alert
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
    bool hasActiveAlert;  // global alert status for the node
};

// packet to and from bytes
size_t serializePacket(const MeshPacket& packet, uint8_t* buffer, size_t maxLen);
bool deserializePacket(const uint8_t* buffer, size_t len, MeshPacket& packet);

// fill existing JsonObject with record data
void nodeRecordToJsonObject(const NodeRecord& record, JsonObject& obj);

// fill existingrecord from a JsonObject (aka, backup)
void jsonObjectToNodeRecord(const JsonObjectConst& obj, NodeRecord& record);

#endif
