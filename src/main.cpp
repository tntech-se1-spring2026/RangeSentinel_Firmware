#include <Arduino.h>
#include "shared_types.h"

// sensor node
#ifdef NODE_TYPE_SENSOR

void setup() {

}

void loop() {

}

#endif

// ----------------------------------

// viewing node
#ifdef NODE_TYPE_VIEWER

#define MAX_NODES 10
NodeStatus networkDatabase[MAX_NODES] = {0};

// helper function to update database
void updateDatabase(NodeStatus incoming) {
    if (incoming.nodeId >= MAX_NODES) {
        return;
    }

    // only update if incoming is newer than what is already there
    if (incoming.messageId > networkDatabase[incoming.nodeId].messageId) {
        networkDatabase[incoming.nodeId] = incoming;
        Serial.printf("DB: Node %d updated (Msg %d)\n", incoming.nodeId, incoming.messageId);
    }
}
// may need more includes here for viewer since it interacts with web server

void setup() {

}

void loop() {

}

#endif
