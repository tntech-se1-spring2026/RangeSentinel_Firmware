> ATTENTION: If you make changes to the schema in some way please update this when it gets merged, so we have a consistent reference

## Data Schema and Persistence
Data is stored in JSON format to be human readable, easily accessible via the web, and light weight enough for the ESP32 to handle efficiently, utilizing `LittleFS.`

### 1. Core Data Structure

The system uses a **Tagged Union** pattern to separate what is sent over LoRa and what is on-device storage.

**A. MeshPacket**
Tailored for LoRa. Contains only the essential sensor data and dynamically packs bytes based on the `DataType`.

```cpp
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
```

**B. NodeRecord** Wraps the packet with metadata that is _not_ sent over radio to save bandwidth. Used for Viewing node and JSON web output.

```cpp
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
```

### 2. JSON Storage & API Formats
In addition to the website files, we hold two distinct files in Flash memory:

`db_backup.json` (Live Status)
Stores the most recent state of each active node.
* Updates every 5 minutes (if a new event occurred)
* Formatted in JSON array of objects:

```json
[
  {
    "id": 1,
    "name": "Front Gate",
    "ls": 154200,
    "mId": 42,
    "alert": false,
    "latched": false,
    "mac": "AA:BB:CC:11:22:33",
    "sensors": [
      { "type": "door", "val": true, "alert": false },
      { "type": "batt", "val": 3.95, "alert": false }
    ]
  }
]
```

`history_backup.json` (Event Log)

Stores the circular buffer history to show an activity feed.
* Updates every 5 minutes (if a new event occurred)
* Formatted in JSON with a metadata header containing the current lcoation of the head and a data array:

```json
{
    "head": 3,
    "data": [
        { 
            "id": 1, 
            "name": "Front Gate", 
            "ls": 150000,
            "mId": 40, 
            "alert": true,
            "latched": true,
            "mac": "AA:BB:CC:11:22:33",
            "sensors": [
                { "type": "motion", "val": true, "alert": true }
            ]
        },
        { 
            "id": 2, 
            "name": "Back Porch", 
            "ls": 152000,
            "mId": 12, 
            "alert": false,
            "latched": false,
            "mac": "11:22:33:AA:BB:CC",
            "sensors": [
                { "type": "door", "val": false, "alert": false },
                { "type": "batt", "val": 4.10, "alert": false }
            ]
        }
    ]
}
```

### 3. Table for Translation from C++ variable to JSON key:

**Meta-Data Fields**
| C++ Variable | JSON Key | Type | Description |
| :--- | :--- | :--- | :--- |
| nodeId | id | int | Unique Node ID | 
| lastPacket.messageId | mId | int | Message number | 
| nodeName | name | string | Name (Stored locally) |
| lastSeen | ls | long | Timestamp since booted in milliseconds |
| lastPacket.readings[] | sensors | array | List of sensor objects |
| hasActiveAlert | alert | bool | True if currently in an alert state |
| alertLatched | latched | bool | True if an alert has been sent but not cleared |
| MACAddress | mac | string | 6-byte hardware MAC converted to a string |

---

**Sensor Fields**
| C++ Variable | JSON Key | Type | Description | 
| :--- | :--- | :--- | :--- | 
| sensorIndex | idx | int | Which sensor on the board | 
| type | type | string | Enum converted to string ("door", "motion", "batt", "error", "assign_id", "assign_mac", "req_assign", "unknown") | 
| payload | val | mixed | value (bool, float, or int depending on type) |
| isAlert | alert | bool | True is this specific reading triggered an alert | 
