#include "shared_types.h"

// squash struct into byte array for LoRa transmission
size_t serializePacket(const MeshPacket& packet, uint8_t* buffer, size_t maxLen) {
    size_t cursor = 0;  // track where we are writing to in byte array
    // need at least 6 bytes for header (NodeID[1] _ MessageID[4] + Count[1])
    if (cursor + 6 > maxLen) {
        return 0;
    }
    buffer[cursor++] = packet.nodeId;  // note post increment
    memcpy(&buffer[cursor], &packet.messageId, 4);  // write messageId
    cursor += 4;
    buffer[cursor++] = packet.readingCount;

    // loop through every sensor in the packet
    for (int i = 0; i < packet.readingCount; i++) {
        if (cursor + 6 > maxLen) break;

        SensorReading r = packet.readings[i];  // get current reading struct
        buffer[cursor++] = r.sensorIndex;
        buffer[cursor++] = (uint8_t)r.type;

        // write actual data
        switch (r.type) {
            // fall down to error case since they would be the same logic
            case SENSOR_TYPE_DOOR:
            case SENSOR_TYPE_MOTION:
            case SENSOR_TYPE_ERROR:
                buffer[cursor++] = r.payload.asByte;
                break;
            case SENSOR_TYPE_BATTERY:
                memcpy(&buffer[cursor], &r.payload.asFloat, 4);
                cursor += 4;
                break;
            default: break;
        }
    }
    return cursor;  // total number of bytes written, needed by LoRa
}

// rebuilds the C++ struct from LoRa bytes
bool deserializePacket(const uint8_t* buffer, size_t len, MeshPacket& packet) {
    size_t cursor = 0;
    if (len < 6) return false;

    packet.nodeId = buffer[cursor++];
    memcpy(&packet.messageId, &buffer[cursor], 4);
    cursor += 4;
    packet.readingCount = buffer[cursor++];

    // loop through how many to read
    for (int i = 0; i < packet.readingCount; i++) {
        if (cursor >= len) break;

        packet.readings[i].sensorIndex = buffer[cursor++];
        packet.readings[i].type = (SensorType)buffer[cursor++];

        switch (packet.readings[i].type) {
            // fall down to error case (all write 1 byte)
            case SENSOR_TYPE_DOOR:
            case SENSOR_TYPE_MOTION:
            case SENSOR_TYPE_ERROR:
                packet.readings[i].payload.asByte = buffer[cursor++];
                // safety check for deserializing booleans
                if (packet.readings[i].type != SENSOR_TYPE_ERROR) {
                    packet.readings[i].payload.asBool = (packet.readings[i].payload.asByte > 0);
                }
                break;
            case SENSOR_TYPE_BATTERY:
                memcpy(&packet.readings[i].payload.asFloat, &buffer[cursor], 4);
                cursor += 4;
                break;
            default: break;
        }
    }
    return true;  // packet successfully parsed
}


// json conversion
void nodeRecordToJsonObject(const NodeRecord& record, JsonObject& obj) {
    // add meta data to json
    obj["id"] = record.lastPacket.nodeId;
    obj["name"] = record.nodeName;
    obj["ls"] = record.lastSeen;
    obj["mId"] = record.lastPacket.messageId;

    JsonArray sensors = obj["sensors"].to<JsonArray>();
    // loop through readings and convert them to JSON objects
    for (int i = 0; i < record.lastPacket.readingCount; i++) {
        // add new obj inside the array
        JsonObject s = sensors.add<JsonObject>();
        SensorReading r = record.lastPacket.readings[i];

        // add sensor index
        s["idx"] = r.sensorIndex;

        // convert enum to humman readable string
        switch (r.type) {
            case SENSOR_TYPE_DOOR:
                s["type"] = "door";
                s["val"] = r.payload.asBool;
                break;
            case SENSOR_TYPE_MOTION:
                s["type"] = "motion";
                s["val"] = r.payload.asBool;
                break;
            case SENSOR_TYPE_BATTERY:
                s["type"] = "batt";
                s["val"] = r.payload.asFloat;
                break;
            case SENSOR_TYPE_ERROR:
                s["type"] = "error";
                s["val"] = r.payload.asByte;
                break;
            default: 
                s["type"] = "unknown";
                break;
        }
    }
}

// fill JSON backup into record
void jsonObjectToNodeRecord(const JsonObjectConst& obj, NodeRecord& record) {
    // restore meta data
    record.lastPacket.nodeId = obj["id"];
    record.lastSeen = obj["ls"];
    record.lastPacket.messageId = obj["mId"];
    strlcpy(record.nodeName, obj["name"] | "Unknown", sizeof(record.nodeName));
}
