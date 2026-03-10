#include "hardware_manager.h"
#include "database_manager.h"

uint8_t nodeID                          = UNASSIGNED_ID;
unsigned long currentMS                 = 0;
const unsigned long oneMinInterval      = 60000;
unsigned long lastHeartBeat             = 0;

// sensor node
#ifdef NODE_TYPE_SENSOR
unsigned long lastRSMS                  = 0; // last reed switch reading
const unsigned long fiftyMSInterval     = 50; 
const unsigned long tenSecInterval      = 10000;
unsigned long lastReq                   = 0; // last requestAssignment function call
SensorType sensor; // holds the type of sensor
void setup() {
    Serial.begin(115200);

    setupRadio(nodeID);

    // TODO: Add logic to decide what kind of sensor node

    // Door logic
    pinMode(RS_PIN, INPUT_PULLUP); // set the reed switch's pin's mode

    // TODO: Cam logic
}

void loop() {
    /*
       IMPORTANT NOTE REGARDING COMMS:
       The loop() needs to run as fast as possible. If you put a delay() 
       at the end of the loop, our node will be "deaf" during that delay.
    */
    currentMS = millis();

    sensorListen();
    
    // requests assignment every 10 seconds while we aren't connected to the network.
    if(nodeID == UNASSIGNED_ID && (currentMS - lastReq > tenSecInterval)){
        lastReq = millis();
        Serial.println("Requesting Assignment");
        sendRequestAssignment();
    }

    // send heartbeat every min ((if its been a minute OR if its hasn't been on for a min & we haven't sent a heartbeat yet) AND we are assigned)
    if((currentMS - lastHeartBeat > oneMinInterval || currentMS < oneMinInterval && lastHeartBeat == 0) && nodeID != UNASSIGNED_ID){
        lastHeartBeat = millis();
        
        while(sendHeartBeat(currentRSState)); // continuously send until it succeeds. 
    }
    
    // reed switch logic
    if(nodeID != UNASSIGNED_ID){
        // delay prevents bouncing
        if(currentMS - lastRSMS > fiftyMSInterval){
            reedSwitchLogic();
        }
    }
    
}
#endif

// --- viewing node ---
#ifdef NODE_TYPE_VIEWER
#include "web_server.h"
#include <DNSServer.h>

static AsyncWebServer server(HTTP_PORT);
unsigned long lastScreenMS      = 0;
unsigned long lastDBMS          = 0;
const long fiveMinInterval      = 300000;
const long fiveSecInterval      = 5000;
String WiFiPassword             = "password";

DNSServer dnsServer;
#define DNS_PORT 53
unsigned long previousMillis = 0;
// holds the time of the last screen update; used to check if we need to update screen again
unsigned long lastScreenUpdate = 0;

void setup(){
    Serial.begin(115200);

    #ifndef WEB_TEST_MODE
        setupScreen();
    #endif

    // initialize the mutex to protect db shared btwn cores
    meshMutex = xSemaphoreCreateMutex();

    // start LittleFS. Halt if failed
    if (!LittleFS.begin(true)){
        Serial.println("An error occurred while mounting LittleFS");
        while(true);
    }

    nodeID = VIEWER_ID; // hard set the nodeID of the viewing node to one
    #ifndef WEB_TEST_MODE
        setupRadio(nodeID);
        xTaskCreatePinnedToCore(
            receiverListen,
            "ReceiverListenTask",
            5000,   // stack size
            NULL,   // parameters
            2,      // priority (higher than loop)
            NULL,   // task handle
            1       // core 1
        );
    #endif

    // start access point
    WiFi.softAP("Range-Sentinel-Gateway", WiFiPassword); // (SSID, Password)
    Serial.print("Access IP Address: ");
    Serial.println(WiFi.softAPIP());  // should default to 192.168.4.1

    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    startWebServer(&server);

    // create NodeStatus for viewer node
    NodeRecord viewerNode;
    uint8_t sensorMAC[6];
    esp_read_mac(sensorMAC, ESP_MAC_WIFI_STA);
    memcpy(viewerNode.MACAddress, sensorMAC, 6);
    viewerNode.lastSeen = millis();
    viewerNode.nodeID = VIEWER_ID;
    memcpy(viewerNode.nodeName, "Viewing Device", 20);

    // create voltage reading
    Reading r;
    r.type = BATTERY_SENSOR;
    r.payload.asFloat = getBatteryVoltage();
    
    // create meshpacket
    MeshPacket fakePacket;
    fakePacket.readingCount = 1;
    fakePacket.messageId = 1;
    fakePacket.readings[0] = r;

    viewerNode.lastPacket = fakePacket;
    
    appendToNetwork(viewerNode);
    numNodesInNetwork++;
    //Serial.println("Viewer node just added to db. numNodes in Network: " + String(numNodesInNetwork));
}

void loop() {
    currentMS = millis();
    
    // update OLED screen
    #ifndef WEB_TEST_MODE
    if(currentMS - lastScreenUpdate > fiveSecInterval){ // if it has been 5 sec since the last screen update
        updateScreen();
        lastScreenUpdate = millis();
    }
    #endif

    dnsServer.processNextRequest();

    // TODO: Add logic that prevents this from updating if there haven't been any changes
    // periodic saving
    if (currentMS - lastDBMS >= fiveMinInterval){
        if(needsPersistence){
            saveDatabaseToFS();
            lastDBMS = currentMS;
        }
    }

    // send heartbeat every min
    if(currentMS - lastHeartBeat > oneMinInterval){
        lastHeartBeat = millis();
        Serial.println("Heartbeat Viewer update");
        viewerHeartBeatUpdate();
        //Serial.println("heartbeat numNodesInNetwork: " + String(numNodesInNetwork));
    }

    ws.cleanupClients();
}

#endif