> ATTENTION: If you make changes to the schema in some way please update this when it gets merged, so we have a consistent reference

## Data Schema and Persistence
Data is stored in JSON format to be human readable, easily accessible via the web, and light weight enough for the ESP32 to handle efficiently, utilizing `LittleFS.`

### 1. Core Data Structure

All sensor data is based on the `NodeStatus` C++ struct.

```cpp
struct NodeStatus {
    uint32_t nodeId;         // Unique identifier for the sensor (1-MAX_NODES)
    uint32_t messageId;      // Incremental number to prevent duplicates
    float batteryVoltage;
    bool motionDetected;
    bool doorOpen;
    char nodeName[20];
    unsigned long lastSeen;  // Relative timestamp (millis) of last contact
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
    "batt": 3.95,
    "motion": false,
    "door": true,
    "name": "Front Gate",
    "ls": 154200
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
        { "id": 1, "mId": 40, "batt": 3.96, "motion": true, "door": false, "name": "Front Gate", "ls": 150000 },
        { "id": 2, "mId": 12, "batt": 4.10, "motion": false, "door": true, "name": "Back Porch", "ls": 152000 }
    ]
}
```

### 3. Table for Translation from C++ variable to JSON key:

| C++ Variable | JSON Key | Type | Description |
| :--- | :--- | :--- | :--- |
| `nodeId` | `id` | `int` | Unique Node ID |
| `messageId` | `mId` | `int` | Message number |
| `batteryVoltage` | `batt` | `float` | Voltage |
| `motionDetected` | `motion` | `bool` | Motion trigger status |
| `doorOpen` | `door` | `bool` | Door status |
| `nodeName` | `name` | `string` | Name |
| `lastSeen` | `ts` | `long` | Timestamp in milliseconds (since booted) |