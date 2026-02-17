#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

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
    uint8_t sensorIndex;    // TODO: REMOVE
    SensorType type;        // format identifier
    SensorData payload;     // actual data
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
    uint8_t MACAddress[6];
    unsigned long lastSeen;  // local timestamp
};

/// @brief 
/// @param packet 
/// @param buffer 
/// @param maxLen 
/// @return 
size_t serializePacket(const MeshPacket& packet, uint8_t* buffer, size_t maxLen);

/// @brief rebuilds the C++ struct from LoRa bytes
/// @param buffer
/// @param len
/// @param packet
/// @return
bool deserializePacket(const uint8_t* buffer, size_t len, MeshPacket& packet);

/// @brief This function fills the JsonObject with record data...
/// @param record 
/// @param obj 
void nodeRecordToJsonObject(const NodeRecord& record, JsonObject& obj);

/// @brief fill existing record from a JsonObject (aka, backup)
/// @param obj 
/// @param record 
void jsonObjectToNodeRecord(const JsonObjectConst& obj, NodeRecord& record);

/// @brief Converts a MAC address string (e.g. "AA:BB:CC:11:22:33") into a 6-byte array.
/// @param MACstr The input string.
/// @param MACRaw A pointer to a uint8_t[6] array to store the result.
void strMACtoRaw(const char* MACstr, uint8_t* MACRaw);

/// @brief Converts a 6-byte MAC array into a formatted string.
/// @param MACRaw Pointer to the 6-byte array.
/// @return A pointer to a static buffer containing the string.
char* rawMACtoStr(uint8_t* MACRaw);
#endif