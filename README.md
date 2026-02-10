> ATTENTION: If you make changes to the schema in some way please update this when it gets merged, so we have a consistent reference

## Data Schema and Persistence
Data is stored in JSON format to be human readable, easily accessible via the web, and light weight enough for the ESP32 to handle efficiently, utilizing `LittleFS.`

### 1. Core Data Structure

The system uses a **Tagged Union** pattern to separate what is sent over LoRa and what is on-device storage.

**A. MeshPacket**
Tailored for LoRa. Contains only the essential sensor data.

```cpp
typedef union {
    bool asBool;        // For Door/Motion
    float asFloat;      // For Battery
    uint8_t asByte;     // For Error Codes
} SensorData;

struct SensorReading {
    uint8_t sensorIndex; // Which sensor is this?
    SensorType type;     // What format is the data? (Door, Battery, etc.)
    SensorData payload;  // The actual data
};

struct MeshPacket {
    uint8_t nodeId;
    uint32_t messageId;
    uint8_t readingCount; 
    SensorReading readings[MAX_SENSORS_PER_PACKET];
};
```

**B. NodeRecord** Wraps the packet with metadata that is _not_ sent over radio to save bandwidth.

```cpp
struct NodeRecord {
    MeshPacket lastPacket;
    char nodeName[20];       // e.g., "Front Gate"
    unsigned long lastSeen;  // Local timestamp (millis)
};
```

### 2. In addition to the website files, we hold two distinct files in Flash memory:

`db_backup.json` (Live Status)
Stores the most recent state of each active node.
* Updates every 5 minutes (if a new event occurred)
* Formatted in JSON array of objects:

```json
[
  {
    "id": 1,
    "mId": 42,
    "name": "Front Gate",
    "ls": 154200,
    "sensors": [
      { "idx": 0, "type": "door", "val": true },
      { "idx": 1, "type": "batt", "val": 3.95 }
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
            "mId": 40, 
            "name": "Front Gate", 
            "ls": 150000,
            "sensors": [
                { "idx": 0, "type": "motion", "val": true }
            ]
        },
        { 
            "id": 2, 
            "mId": 12, 
            "name": "Back Porch", 
            "ls": 152000,
            "sensors": [
                { "idx": 0, "type": "door", "val": true },
                { "idx": 1, "type": "batt", "val": 4.10 }
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
| messageId | mId | int | Message number | 
| nodeName | name | string | Name (Stored locally) |
| lastSeen | ls | long | Timestamp since booted in milliseconds |
| readings[] | sensors | array | List of sensor objects |

---

**Sensor Fields**
| C++ Variable | JSON Key | Type | Description | 
| :--- | :--- | :--- | :--- | 
| sensorIndex | idx | int | Which sensor on the board | 
| type | type | string | Enum converted to string ("door", "batt", "motion", "temp", "err") | 
| payload | val | mixed | value (bool, float, or int depending on type) |