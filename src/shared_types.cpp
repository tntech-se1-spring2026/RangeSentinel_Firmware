#include <ArduinoJson.h>
#include "shared_types.h"

// pack a struct into an existing JsonObject
void nodeToJsonObject(const NodeStatus& status, JsonObject& obj) {
    obj["id"] = status.nodeId;
    obj["mId"] = status.messageId;
    obj["batt"] = status.batteryVoltage;
    obj["motion"] = status.motionDetected;
    obj["door"] = status.doorOpen;
    obj["name"] = status.nodeName;
    obj["ls"] = status.lastSeen;
}

// unpack a JsonObject into an existing struct
void jsonObjectToNode(const JsonObjectConst& obj, NodeStatus& status) {
    status.nodeId = obj["id"];
    status.messageId = obj["mId"];
    status.batteryVoltage = obj["batt"];
    status.motionDetected = obj["motion"];
    status.doorOpen = obj["door"];
    strlcpy(status.nodeName, obj["name"] | "Unknown", sizeof(status.nodeName));   // default to Unknown
    status.lastSeen = obj["ls"];
}

// translate C++ struct to JSON
// single node use
String nodeStatusToJson(const NodeStatus& status) {
    JsonDocument doc;
    JsonObject obj = doc.to<JsonObject>();
    nodeToJsonObject(status, obj);
    String output;
    serializeJson(doc, output);
    return output;
}
