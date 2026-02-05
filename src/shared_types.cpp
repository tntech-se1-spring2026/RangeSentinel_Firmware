#include <ArduinoJson.h>
#include "shared_types.h"

// translate C++ struct to JSON
String nodeStatusToJson(const NodeStatus& status) {
    JsonDocument doc;

    // map database entry (struct) to API fields in Json
    doc["id"] = status.nodeId;
    doc["mId"] = status.messageId;
    doc["batt"] = status.batteryVoltage;
    doc["motion"] = status.motionDetected;
    doc["door"] = status.doorOpen;
    doc["name"] = String(status.nodeName);
    doc["ts"] = status.lastSeen;

    String output;
    serializeJson(doc, output);

    return output;
}
