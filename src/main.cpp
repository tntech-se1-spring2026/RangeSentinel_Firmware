#include <Arduino.h>
#include <ArduinoJson.h>
#include <array>
#include "shared_types.h"
#include "web_server.h"
#include "database_manager.h"

#include <SPI.h> // translator for SX1276 LoRa chip
#include <RH_RF95.h> // The physical layer driver (SX1276)
#include <RHMesh.h> // The network layer manager (Routing/Mesh)

// --- PIN DEFINITIONS ---
// To be filled out when we get this wired up. Specifically for the devboard w/ separate radio transceiver
#define RFM95_CS    0  // Chip Select (NSS)
#define RFM95_INT   0  // Interrupt (DIO0)
#define RFM95_RST   0  // Reset pin

u8_t nodeID = 0; // the ID for this node

RH_RF95 rf95(RFM95_CS, RFM95_INT); // radio driver
RHMesh manager(rf95, nodeID); // class to manage comm routing.

#pragma region SHARED_FUNCTIONS
void setupRadio(){
    // Manual reset of the LoRa radio to ensure a clean state
    Serial.println("Reseting radio");
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
    delay(100);
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(100);

    // Initialize the Mesh Manager & LoRa driver (rf95)
    if (!manager.init()){
        Serial.println("Mesh init failed! Check wiring.");
        while(true); // Halt if hardware isn't responding
    }

    // set Radio frequency
    if (!rf95.setFrequency(915.0)){
        Serial.println("setFrequency failed");
    }

    // TX Power: 5 to 23 dBm. 23 is max power. 
    // false = don't use RFO pin, use PA_BOOST (standard for SX1276)
    rf95.setTxPower(5, false);
    
    Serial.println("Mesh Node Online. ID: " + String(nodeID));
}
#pragma endregion

// sensor node
#ifdef NODE_TYPE_SENSOR
void listenFunction(){ // Must be called constantly to process incoming packets
    uint8_t incoming[RH_MESH_MAX_MESSAGE_LEN]; // buffer to hold the raw bytes of any incoming message.
    uint8_t len = sizeof(incoming);
    uint8_t fromAddress;

    // recvfromAck returns true if addressed for us, if not returns false & forwards to appropriate node.
    if (manager.recvfromAck(incoming, &len, &fromAddress)){
        Serial.println("Message received from " + String(fromAddress));
        incoming[len] = '\0'; // turn the raw byte array into a String

        // store incoming message in json format
        JsonDocument incomingDoc;
        DeserializationError err = deserializeJson(incomingDoc, incoming);

        if(err){
            Serial.print("JSON Parse failed: ");
            Serial.println(err.c_str());
        }else{
            bool found = false;
            for(NodeStatus &node : networkDatabase){ // check each node
                if(node.nodeId == fromAddress){ // if node matches, update our db
                    found = true;
                    // update db
                    node.messageId = incomingDoc["mId"].as<long>();
                    node.batteryVoltage = incomingDoc["batt"].as<long>();
                    node.motionDetected = incomingDoc["motion"].as<bool>();
                    node.doorOpen = incomingDoc["door"].as<bool>();
                    node.nodeName = incomingDoc["name"].as<String>();

                    // exit loop
                    break;
                }
            }
            if(!found){ // if our sending node wasn't found in our db
                // add new node logic maybe? need to think this out more
                Serial.println("Sending node not found in db");
            }
        }
    }
}

void setup() {
    setupRadio();
}

void loop() {
    /*
       IMPORTANT NOTE REGARDING COMMS:
       The loop() needs to run as fast as possible. If you put a delay() 
       at the end of the loop, our node will be "deaf" during that delay.
    */
    listenFunction();
}

#endif

// ----------------------------------
// viewing node
#ifdef NODE_TYPE_VIEWER
#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

// standard HTTP port
WebServer server(80);
std::array<NodeStatus, MAX_NODES> networkDatabase = {};

void receiverNodeListenFunction(void* pvParameters) {
    while(true){
        uint8_t incoming[RH_MESH_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(incoming);
        uint8_t fromAddress;

        // Listen for mesh traffic
        if (manager.recvfromAck(incoming, &len, &fromAddress)) {
            Serial.println("Message received from " + String(fromAddress));
            incoming[len] = '\0';
            
            // LOCK
            if (xSemaphoreTake(meshMutex, portMAX_DELAY)) {
                JsonDocument incomingDoc;

                DeserializationError err = deserializeJson(incomingDoc, incoming);
                if(err){
                    Serial.print("JSON parse failed: ");
                    Serial.println(err.c_str());
                }else{
                    bool found = false;
                    for(NodeStatus &node : networkDatabase){ // check each node
                        if(node.nodeId == fromAddress){ // if node matches, update our db
                            found = true;
                            // update db
                            node.messageId = incomingDoc["mId"].as<long>();
                            node.batteryVoltage = incomingDoc["batt"].as<long>();
                            node.motionDetected = incomingDoc["motion"].as<bool>();
                            node.doorOpen = incomingDoc["door"].as<bool>();
                            node.nodeName = incomingDoc["name"].as<String>();

                            // exit loop
                            break;
                        }
                    if(!found){ // if our sending node wasn't found in our db
                    }
                        // add new node logic maybe? need to think this out more
                        Serial.println("Sending node not found in db");
                    }
                }
            }
                xSemaphoreGive(meshMutex); // UNLOCK
            
            Serial.print("Mesh data synced to Web Server from: ");
            Serial.println(fromAddress);

        }
        // FEED THE WATCHDOG: This 1ms pause is mandatory on Core 0
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void setup() {
    // set speed
    Serial.begin(115200);

    // initialize the mutex to protect db shared btwn cores
    meshMutex = xSemaphoreCreateMutex();

    setupRadio();

    // start LittleFS. Halt if failed
    if (!LittleFS.begin(true)){
        Serial.println("An error occurred while mounting LittleFS");
        while(true);
    }
    Serial.println("LittleFS mounted successfully");

    // start access point
    // (SSID, Password)
    WiFi.softAP("Range-Sentinel-Gateway", "secure-sentinel-2026");
    Serial.print("Access IP Address: ");
    Serial.println(WiFi.softAPIP());  // should default to 192.168.4.1

    setupWebServer(getDatabaseAsJson);

    // run the listen function on core 0
    xTaskCreatePinnedToCore(
        receiverNodeListenFunction,
        "ReceiverListenTask",
        1000,
        NULL,
        1,
        NULL,
        0
    );
}

void loop() {
    server.handleClient();
}

#endif
