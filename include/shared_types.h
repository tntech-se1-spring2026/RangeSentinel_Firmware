#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <ArduinoJson.h>

#define MAX_SENSORS_PER_PACKET 4
#define MAX_PACKET_SIZE RMT_MEM_64
#define BATTERY_THRESHOLD 3.4  // voltage considered to be low battery for alert

typedef uint8_t MacAddress[6];

// what kind of data?

typedef uint8_t MacAddress[6];

typedef enum {
    OTHER               = 0x00, 
    DOOR_SENSOR         = 0x01, // sends open as bool
    MOTION_SENSOR       = 0x02, // sends motion as bool
    BATTERY_SENSOR      = 0x03, // sends voltage as float
    ASSIGNMENT_ID       = 0x04, // sends nodeID as byte
    ASSIGNMENT_MAC      = 0x05, // sends MAC as byte  
    REQUEST_TO_ASSIGN   = 0x06, // sends MAC as byte
    SENSOR_TYPE_ERROR   = 0xFF
} DataType;

// payload holder
typedef union {
    bool asBool;
    float asFloat;
    uint8_t asByte;
    MacAddress asMAC;
    MacAddress asMAC;
} Data;

// single sensor event
struct Reading {
    //uint8_t sensorIndex;      // currently unused; will be used if we have multiple of the same sensors (if used uncomment usage in serialization functions)
    DataType type;              // format identifier
    Data payload;               // actual data
    bool isAlert;
};

// what is sent over LoRa
struct MeshPacket {
    uint32_t messageId;
    uint8_t readingCount;
    Reading readings[MAX_SENSORS_PER_PACKET];
};

// metadata not sent over radio (will be stored in viewing node for comparison)
struct NodeRecord {
    uint8_t nodeID;
    MeshPacket lastPacket;
    char nodeName[20];
    uint8_t MACAddress[6];
    unsigned long lastSeen;  // local timestamp
    bool hasActiveAlert;  // global alert status for the node (live status)
    bool alertLatched;  // tracks if an alert hasn't been cleared yet
};

/// @brief Converts a MeshPacket struct into a compact byte array for LoRa transmission
/// @param packet The source MeshPacket object containing the data to be transmitted
/// @param buffer The destination byte array where the serialized data will be stored
/// @param maxLen The size of the destination buffer to prevent overflow
/// @return The actual number of bytes written to the buffer
size_t serializePacket(const MeshPacket& packet, uint8_t* buffer, size_t maxLen);

/// @brief Reconstructs a MeshPacket struct from raw bytes received from LoRa
/// @param buffer Pointer to the raw data received by LoRa
/// @param len The number of bytes available in the buffer to be parsed
/// @param packet The MeshPacket struct to be populated with the deserialized data
/// @return True if the data was successfully parsed; False otherwise
bool deserializePacket(const uint8_t* buffer, size_t len, MeshPacket& packet);

/// @brief Converts a NodeRecord into ArduinoJson object for API or persistent storage
/// @param record The read only source NodeRecord
/// @param obj The JsonObject to be populated with the record's key-value pairs
void nodeRecordToJsonObject(const NodeRecord& record, JsonObject& obj);

/// @brief Fills in a NodeRecord struct using data from a JSON object (e.g., loading from backup file)
/// @param obj The read only source JSON data
/// @param record The NodeRecord struct to be updated with the JSON values
void jsonObjectToNodeRecord(const JsonObjectConst& obj, NodeRecord& record);

/// @brief Converts a MAC address string (e.g. "AA:BB:CC:11:22:33") into a 6-byte array.
/// @param MACstr The input string.
/// @param MACRaw A pointer to a uint8_t[6] array to store the result.
void strMACtoRaw(const char* MACstr, uint8_t* MACRaw);

/// @brief Converts a 6-byte MAC array into a formatted string.
/// @param MACRaw Pointer to the 6-byte array.
/// @return A pointer to a static buffer containing the string.
char* rawMACtoStr(uint8_t* MACRaw);

/// @brief this function returns the first reading of a particular type in the readings array.
/// @param readings the array of readings in the mesh packet
/// @param type the desired type of reading to be returned
/// @return returns the reading of the type passed. If not found, returns nullptr
Reading* getReadingOfType(Reading (&readings)[4], DataType type);
#endif