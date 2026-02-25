#include "shared_types.h"

size_t serializePacket(const MeshPacket& packet, uint8_t* buffer, size_t maxLen) {
    size_t cursor = 0;  // track where we are writing to in byte array
    // need at least 6 bytes for header (NodeID[1] _ MessageID[4] + Count[1])
    if (cursor + 6 > maxLen) {
        return 0;
    }
    memcpy(&buffer[cursor], &packet.messageId, 4);  // write messageId
    cursor += 4;
    buffer[cursor++] = packet.readingCount;

    // loop through every sensor in the packet
    for (int i = 0; i < packet.readingCount; i++) {
        // max reading size is 7  bytes (1 for type + 6 for MAC)
        if (cursor + 7 > maxLen) break;

        Reading r = packet.readings[i];  // get current reading struct
        //buffer[cursor++] = r.sensorIndex;
        buffer[cursor++] = (uint8_t)r.type;

        // write actual data
        switch (r.type) {
            // fall down to error case since they would be the same logic
            // 1 byte
            case DOOR_SENSOR:
            case MOTION_SENSOR:
            case ASSIGNMENT_ID:
            case SENSOR_TYPE_ERROR:
            if (cursor + 1 > maxLen) break;
                buffer[cursor++] = r.payload.asByte;
                break;
            // 4 bytes
            case BATTERY_SENSOR:
            if (cursor + 4 > maxLen) break;
                memcpy(&buffer[cursor], &r.payload.asFloat, 4);
                cursor += 4;
                break;
            // 6 bytes
            case ASSIGNMENT_MAC:
            case REQUEST_TO_ASSIGN:
            if (cursor + 6 > maxLen) break;
                memcpy(&buffer[cursor], &r.payload.asMAC, 6);
                cursor += 6;
                break;
            case OTHER:
            default: break;
        }
    }
    return cursor;  // total number of bytes written, needed by LoRa
}

bool deserializePacket(const uint8_t* buffer, size_t len, MeshPacket& packet) {
    size_t cursor = 0;
    if (len < 6) return false;

    memcpy(&packet.messageId, &buffer[cursor], 4);
    cursor += 4;
    packet.readingCount = buffer[cursor++];

    // loop through how many to read
    for (int i = 0; i < packet.readingCount; i++) {
        if (cursor >= len) break;

        packet.readings[i].type = (DataType)buffer[cursor++];

        switch (packet.readings[i].type) {
            // fall down to error case (all write 1 byte)
            case DOOR_SENSOR:
            case MOTION_SENSOR:
            case ASSIGNMENT_ID:
            case SENSOR_TYPE_ERROR:
                if (cursor + 1 > len) return false;
                packet.readings[i].payload.asByte = buffer[cursor++];
                // safety check for deserializing booleans
                if (packet.readings[i].type == DOOR_SENSOR || packet.readings[i].type == MOTION_SENSOR) {
                    packet.readings[i].payload.asBool = (packet.readings[i].payload.asByte > 0);
                }
                break;
            // write 4 bytes
            case BATTERY_SENSOR:
            if (cursor + 4 > len) return false;
                memcpy(&packet.readings[i].payload.asFloat, &buffer[cursor], 4);
                cursor += 4;
                break;
            // write 6 bytes
            case ASSIGNMENT_MAC:
            case REQUEST_TO_ASSIGN:
                //bounds check
                if (cursor + 6 > len) return false;
                memcpy(packet.readings[i].payload.asMAC, &buffer[cursor], 6);
                cursor += 6;
                break;
            case OTHER:
            default: break;
        }
    }
    return true;  // packet successfully parsed
}

void nodeRecordToJsonObject(const NodeRecord& record, JsonObject& obj) {
    // add meta data to json
    obj["id"] = record.nodeID;
    obj["name"] = record.nodeName;
    obj["ls"] = record.lastSeen;
    obj["mId"] = record.lastPacket.messageId;
    obj["alert"] = record.hasActiveAlert;
    obj["latched"] = record.alertLatched;
    obj["mac"] = rawMACtoStr(record.MACAddress);

    JsonArray sensors = obj["sensors"].to<JsonArray>();
    // loop through readings and convert them to JSON objects
    for (int i = 0; i < record.lastPacket.readingCount; i++) {
        // add new obj inside the array
        JsonObject s = sensors.add<JsonObject>();
        Reading r = record.lastPacket.readings[i];

        // add sensor index
        //s["idx"] = r.sensorIndex;

        // convert enum to human readable string
        switch (r.type) {
            case DOOR_SENSOR:
                s["type"] = "door";
                s["val"] = r.payload.asBool;
                s["alert"] = r.isAlert;
                break;
            case MOTION_SENSOR:
                s["type"] = "motion";
                s["val"] = r.payload.asBool;
                s["alert"] = r.isAlert;
                break;
            case BATTERY_SENSOR:
                s["type"] = "batt";
                s["val"] = r.payload.asFloat;
                s["alert"] = r.isAlert;
                break;
            case SENSOR_TYPE_ERROR:
                s["type"] = "error";
                s["val"] = r.payload.asByte;
                s["alert"] = r.isAlert;
                break;
            case ASSIGNMENT_ID:
                s["type"] = "assign_id";
                s["val"] = r.payload.asByte;
                s["alert"] = r.isAlert;
                break;
            case ASSIGNMENT_MAC:
                s["type"] = "assign_mac";
                s["val"] = rawMACtoStr(r.payload.asMAC);
                s["alert"] = r.isAlert;
                break;
            case REQUEST_TO_ASSIGN:
                s["type"] = "req_assign";
                s["val"] = rawMACtoStr(r.payload.asMAC);
                s["alert"] = r.isAlert;
                break;
            default: 
                s["type"] = "unknown";
                break;
        }
    }
}

void jsonObjectToNodeRecord(const JsonObjectConst& obj, NodeRecord& record) {
    // restore meta data
    record.nodeID = obj["id"];
    record.lastSeen = obj["ls"];
    record.lastPacket.messageId = obj["mId"];
    strlcpy(record.nodeName, obj["name"] | "Unknown", sizeof(record.nodeName));
    record.hasActiveAlert = obj["alert"] | false;
    record.alertLatched = obj["latched"] | false;
    strMACtoRaw(obj["mac"], record.MACAddress);

    // restore sensor readings
    JsonArrayConst sensors = obj["sensors"];
    int count = 0;

    for (JsonObjectConst s : sensors) {
        // don't overflow readings array
        if (count >= MAX_SENSORS_PER_PACKET) break;

        const char* typeStr = s["type"];
        Reading& r = record.lastPacket.readings[count];
        r.isAlert = s["alert"] | false;

        if (strcmp(typeStr, "door") == 0) {
            r.type = DOOR_SENSOR;
            r.payload.asBool = s["val"];
        }
        else if (strcmp(typeStr, "motion") == 0) {
            r.type = MOTION_SENSOR;
            r.payload.asBool = s["val"];
        }
        else if (strcmp(typeStr, "batt") == 0) {
            r.type = BATTERY_SENSOR;
            r.payload.asFloat = s["val"];
        }
        else if (strcmp(typeStr, "assign_id") == 0) {
            r.type = ASSIGNMENT_ID;
            r.payload.asByte = s["val"];
        }
        else if (strcmp(typeStr, "assign_mac") == 0) {
            r.type = ASSIGNMENT_MAC;
            strMACtoRaw(s["val"], r.payload.asMAC);
        }
        else if (strcmp(typeStr, "req_assign") == 0) {
            r.type = REQUEST_TO_ASSIGN;
            strMACtoRaw(s["val"], r.payload.asMAC);
        }
        else if (strcmp(typeStr, "error") == 0) {
            r.type = SENSOR_TYPE_ERROR;
            r.payload.asByte = s["val"];
        }
        else {
            r.type = OTHER;
        }
    }
    record.lastPacket.readingCount = count;
}

void strMACtoRaw(const char* MACstr, uint8_t* MACRaw) {
    sscanf(MACstr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
           &MACRaw[0], &MACRaw[1], &MACRaw[2], 
           &MACRaw[3], &MACRaw[4], &MACRaw[5]);
}

char* rawMACtoStr(const uint8_t* MACRaw) {
    static char buffer[18]; // "XX:XX:XX:XX:XX:XX\0" is 18 chars
    snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X:%02X:%02X:%02X",
            MACRaw[0], MACRaw[1], MACRaw[2], 
            MACRaw[3], MACRaw[4], MACRaw[5]);
    return buffer;
}

Reading* getReadingOfType(Reading (&readings)[4], DataType type){
    for (Reading& reading : readings) {
        if (reading.type == type) {
            return &reading;   // found
        }
    }
    return nullptr;            // not found
}
