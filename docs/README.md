# Range Sentinel Firmware

> ATTENTION: If you make changes to the schema in some way please update this when it gets merged, so we have a consistent reference.

Data Schema and Persistence

We have a separation between internal persistent data (how the ESP32 saves its state) and Web API data (what the frontend dashboard sees).

# 1. Core Data Structure

The system uses a Tagged Union pattern to separate what is sent over LoRa and what is stored on-device.

### A. MeshPacket
Tailored for LoRa. Contains only the essential sensor data and dynamically packs bytes based on the DataType.

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
    DataType type;              // format identifier
    Data payload;               // actual data
    bool isAlert;               // evaluated on receipt
};

// what is sent over LoRa
struct MeshPacket {
    uint32_t messageId;
    uint8_t readingCount;
    Reading readings[MAX_SENSORS_PER_PACKET];
};
```

### B. NodeRecord Wraps the packet with metadata that is not sent over radio to save bandwidth. Used for the Viewing node's internal database, LittleFS backups, and as the source data for the Web API.

```cpp
struct NodeRecord {
    uint8_t nodeID;
    MeshPacket lastPacket;
    char nodeName[20];
    uint8_t MACAddress[6];
    unsigned long lastSeen;  // local timestamp
    bool hasActiveAlert;     // global alert status for the node (live status)
    bool alertLatched;       // tracks if an alert hasn't been cleared yet
};
```

# 2. Persistent Storage (LittleFS)

We hold two distinct files in Flash memory to ensure system state survives power cycles.

### db_backup.json (Live Status)
Stores the complete, raw state of each active node, including sensitive metadata required for network operations.

Updates when a new event occurs or a setting is changed.

Formatted as a JSON array of objects:

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

### history_backup.json (Event Log)
Stores the circular buffer history to show an activity feed.

Formatted in JSON with a metadata header containing the current location of the head and a data array.

```cpp
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
        }
    ]
}
```

# 3. Web API Endpoints

These endpoints are specifically tailored for the frontend dashboard. They filter out internal metadata (like MAC addresses) and convert raw values into human-readable formats.

### GET /web/nodes (Live Dashboard Data)
Provides the live status of all nodes. Automatically calculates connection status and formats sensor outputs.

```json
[
  {
    "id": 1,
    "name": "Front Gate",
    "type": "sensor",
    "status": "Online",
    "sensors": [
      { "type": "battery", "value": 97.12 },
      { "type": "door", "value": "Open" }
    ]
  }
]
```

### GET /web/alerts (Active Notifications)
A minimalist endpoint used by the frontend to poll for unacknowledged alerts. Only returns nodes where alertLatched is true.

```json
[
  {
    "id": 1,
    "name": "Front Gate",
    "time": 154200,
    "reasons": ["Door Opened", "Low Battery"]
  }
]
```

### POST /web/rename (Update Node Name)
Updates a node's custom name and immediately triggers a database backup to persistent storage.

Params: id (integer), name (string limit 20 chars).

### POST /web/ack (Acknowledge Alert)
Clears the alertLatched flag for a specific node, removing it from the alerts endpoint.

Params: id (integer).

# 4. Schema Translation Tables

### A. Internal Persistence Translation (db_backup.json)
| C++ Variable | JSON Key | Type | Description |
| :--- | :--- | :--- | :--- |
| nodeId | id | int | Unique Node ID |
| lastPacket.messageId | mId | int | Message number |
| nodeName | name | string | Custom Name |
| lastSeen | ls | long | Timestamp since booted in milliseconds |
| hasActiveAlert | alert | bool | True if currently in an alert state |
| alertLatched | latched | bool | True if an alert has been sent but not cleared |
| MACAddress | mac | string | 6-byte hardware MAC converted to a string |
| type | type | string | Enum converted to string ("door", "motion", "batt", etc.) |
| payload | val | mixed | Raw value (bool, float, or int depending on type) |

### B. Web API Specific Fields (/web/nodes)
| JSON Key | Source / Logic | Description |
| :--- | :--- | :--- |
| type | nodeID == 1 ? "viewing" : "sensor" | Categorizes the node for UI icons |
| status | millis() - lastSeen < 30000 | "Online" if seen within 30s, else "Offline" |
| value | getBatteryPercentageFromV() | Battery voltage automatically converted to 0-100% |
| value | asBool ? "Closed" : "Open" | Door state converted to a readable string |