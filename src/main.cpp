#include "hardware_manager.h"
#include "database_manager.h"

uint32_t nodeID                 = 0; // zero is the nodeID that all sensor nodes get set to while waiting to be assigned as a node in the mesh from the viewer node
unsigned long currentMS         = 0;

// sensor node
#ifdef NODE_TYPE_SENSOR
unsigned long lastRSMS          = 0;
const long fiftyMSInterval      = 50;
void setup() {
    setupRadio(0);
    // TODO: Add logic to decide what kind of sensor node
    pinMode(RS_PIN, INPUT_PULLUP); // set the reed switch's pin's mode
}

void loop() {
    /*
       IMPORTANT NOTE REGARDING COMMS:
       The loop() needs to run as fast as possible. If you put a delay() 
       at the end of the loop, our node will be "deaf" during that delay.
    */
   // TODO: create sensor node logic
    currentMS = millis();
    
    sensorListen();
    
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

    // initialize the mutex to protect db shared btwn cores
    meshMutex = xSemaphoreCreateMutex();
    
    setupRadio(nodeID);

    // start LittleFS. Halt if failed
    if (!LittleFS.begin(true)){
        Serial.println("An error occurred while mounting LittleFS");
        while(true);
    }

    getDatabaseFromFS();

    // start access point
    WiFi.softAP("Range-Sentinel-Gateway", WiFiPassword); // (SSID, Password)
    Serial.print("Access IP Address: ");
    Serial.println(WiFi.softAPIP());  // should default to 192.168.4.1

    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    startWebServer(&server);

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
}

void loop() {
    currentMS = millis();
    
    // update OLED screen
    if(currentMS - lastScreenUpdate > fiveSecInterval){ // if it has been 5 sec since the last screen update
        updateScreen();
        lastScreenUpdate = millis();
    }

    // TODO: Add logic that prevents this from updating if there haven't been any changes
    dnsServer.processNextRequest();
    // periodic saving
    if (currentMS - lastDBMS >= fiveMinInterval){
        if(needsPersistence){
            saveDatabaseToFS();
            lastDBMS = currentMS;
        }
    }
}

#endif