#include "hardware_manager.h"
#include "database_manager.h"

uint8_t nodeID                         = UNASSIGNED_ID; // 254 is the nodeID that all sensor nodes get set to while waiting to be assigned as a node in the mesh from the viewer node
unsigned long currentMS                 = 0;

// sensor node
#ifdef NODE_TYPE_SENSOR
unsigned long lastRSMS                  = 0; // last reed switch reading
const unsigned long fiftyMSInterval     = 50; 
const unsigned long oneMinInterval      = 60000;
const unsigned long tenSecInterval      = 10000;
unsigned long lastReq                   = 0; // last requestAssignment function call
unsigned long lastHeartBeat             = 0;
SensorType sensor; // holds the type of sensor
void setup() {
    Serial.begin(115200);
    setupRadio(nodeID);

    // TODO: Add logic to decide what kind of sensor node

    // TODO: door logic
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
        requestAssignment();
    }

    // send heartbeat every min
    if(currentMS - lastHeartBeat > oneMinInterval){
        lastHeartBeat = millis();
        sendHeartBeat();
    }
    
    // delay prevents bouncing
    // if(currentMS - lastRSMS > fiftyMSInterval){
    //     reedSwitchLogic();
    // }
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

    setupScreen();

    // start LittleFS. Halt if failed
    if (!LittleFS.begin(true)){
        Serial.println("An error occurred while mounting LittleFS");
        while(true);
    }
    getDatabaseFromFS();

    // initialize the mutex to protect db shared btwn cores
    meshMutex = xSemaphoreCreateMutex();
    
    nodeID = VIEWER_ID; // hard set the nodeID of the viewing node to one
    setupRadio(nodeID);
    // run the listen function exclusively on core 0
    xTaskCreatePinnedToCore(
        receiverListen,
        "ReceiverListenTask",
        5000,
        NULL,
        1,
        NULL,
        0
    );

    // start access point
    WiFi.softAP("Range-Sentinel-Gateway", WiFiPassword); // (SSID, Password)
    Serial.print("Access IP Address: ");
    Serial.println(WiFi.softAPIP());  // should default to 192.168.4.1

    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    startWebServer(&server);
}

void loop() {
    currentMS = millis();
    
    // update OLED screen
    if(currentMS - lastScreenUpdate > fiveSecInterval){ // if it has been 5 sec since the last screen update
        updateScreen();
        lastScreenUpdate = millis();
    }

    dnsServer.processNextRequest();

    // TODO: Add logic that prevents this from updating if there haven't been any changes
    // periodic saving
    if (currentMS - lastDBMS >= fiveMinInterval){
        if(needsPersistence){
            saveDatabaseToFS();
            lastDBMS = currentMS;
        }
    }
}

#endif