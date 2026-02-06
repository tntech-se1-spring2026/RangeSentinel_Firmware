#include <Arduino.h>
#include <ArduinoJson.h>
#include <array>

#include "shared_types.h"
#include "web_server.h"
#include "database_manager.h"
#include "pins.h" // has all our physical pins

#include <Wire.h> // for low level comm w/screen
#include <Adafruit_GFX.h> // graphics for screen
#include <Adafruit_SSD1306.h> // driver chip for screen

#include <SPI.h> // translator for SX1276 LoRa chip
#include <RH_RF95.h> // The physical layer driver (SX1276)
#include <RHMesh.h> // The network layer manager (Routing/Mesh)

// zero is the nodeID that all sensor nodes get set to while waiting to be assigned as a node in the mesh from the viewer node
u8_t nodeID = 0; // the ID for this node

// --- RADIO COMM ---
RH_RF95 rf95(RFM95_CS, RFM95_INT); // radio driver
RHMesh* manager; // obj to manage mesh comm routing.

// --- SCREEN ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#pragma region SHARED_FUNCTIONS
void setupRadio(){
    // Manual reset of the LoRa radio to ensure a clean state
    Serial.println("Reseting radio");
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, LOW);  // Pull low to reset
    delay(10); 
    digitalWrite(RFM95_RST, HIGH); // Pull high to operate
    delay(10);

    manager = new RHMesh(rf95, nodeID);

    // Initialize the Mesh Manager & LoRa driver (rf95)
    if (!manager->init()){
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

void setupScreen(){
    // OLED Pins for V2.1_1.6 are SDA: 21, SCL: 22
    Wire.begin(21, 22); 
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("OLED failed to initialize"));
        while(true);
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    analogReadResolution(12); // ESP32 ADC is 12-bit (0-4095)
}

float getBatteryVoltage(){
    #define ADC_PIN 35 // Battery is hardwired to GPIO 35 on our T3_v1.6.1 board

    uint16_t raw = analogRead(ADC_PIN); // Read Raw ADC value

    // Convert and return voltage
    // The board uses a divider that halves the voltage, so we multiply by 2.
    // 3.3V / 4095 units * 2 (divider) * 1.1 (typical ESP32 calibration)
    return ((raw / 4095.0) * 3.3 * 2.0 * 1.1);
}

int getBatteryPercentage(){
    // Li-ion range: 4.2V (100%) down to 3.2V (0%)
    int percentage = (int)((getBatteryVoltage() - 3.2) / (4.2 - 3.2) * 100);
    return(constrain(percentage, 0, 100));
}

void updateScreen(){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Range Sentinel Receiver");

    display.setTextSize(2);
    display.setCursor(0, 25);
    display.printf("%d%%", getBatteryPercentage());

    display.setTextSize(1);
    display.setCursor(0, 50);
    display.printf("Voltage: %.2fV", getBatteryVoltage());

    display.display();
}
#pragma endregion

// sensor node
#ifdef NODE_TYPE_SENSOR
void listenFunction(){ // Must be called constantly to process incoming packets
    uint8_t incoming[RH_MESH_MAX_MESSAGE_LEN]; // buffer to hold the raw bytes of any incoming message.
    uint8_t len = sizeof(incoming);
    uint8_t fromAddress;

    // recvfromAck returns true if addressed for us, if not returns false & forwards to appropriate node.
    if (manager->recvfromAck(incoming, &len, &fromAddress)){
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
                    strlcpy(node.nodeName, incomingDoc["name"].as<const char*>(), sizeof(node.nodeName));

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
    switch (nodeID){
        case 0: // if our node is unassigned
            /* code */
            manager.setThisAddress(0);
        break;
        default: // if our node has been assigned
            listenFunction();
        break;
   }
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

// holds the time of the last screen update
unsigned long lastScreenUpdate = 0;

void assignNewNodeID(const char* macStr){
    uint8_t tempMac[6];
    
    // Convert string "AA:BB..." to raw bytes
    sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
           &tempMac[0], &tempMac[1], &tempMac[2], 
           &tempMac[3], &tempMac[4], &tempMac[5]);

    uint8_t assignedID = 0;


    // Check if MAC exists in networkDatabase
    for(NodeStatus &node : networkDatabase){
        if(memcmp(node.nodeMACAddress, tempMac, 6) == 0){
            assignedID = node.nodeId;
            break;
        }
    }
    // If not, find next ID and appendToNetwork
    // Send response back to Node 0 via manager->sendtoWait
}

void receiverNodeListenFunction(void* pvParameters){
    while(true){
        uint8_t incoming[RH_MESH_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(incoming);
        uint8_t fromAddress;

        // Listen for mesh traffic
        if (manager->recvfromAck(incoming, &len, &fromAddress)) {
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
                    if(fromAddress == 0){ // if this is a new/unassigned node
                        const char* incomingMac = incomingDoc["mac"] | ""; 
                        if (strlen(incomingMac) > 0) {
                            assignNewNodeID(incomingMac);
                        }
                    }else{ // not unassigned
                        bool found = false;
                        for(NodeStatus &node : networkDatabase){ // check each node in db to see if matches
                            if(node.nodeId == fromAddress){ // if node matches, update our db
                                // Verify MAC address
                                // if(){
                                    found = true;
                                    // update db
                                    node.messageId = incomingDoc["mId"].as<long>();
                                    node.batteryVoltage = incomingDoc["batt"].as<float>();
                                    node.motionDetected = incomingDoc["motion"].as<bool>();
                                    node.doorOpen = incomingDoc["door"].as<bool>();
                                    strlcpy(node.nodeName, incomingDoc["name"].as<const char*>(), sizeof(node.nodeName));
                                    // exit loop
                                    break;
                                // }
                            }
                        }
                        if(!found){ // if our sending node wasn't found in our db
                            Serial.println("Warning: Unrecognized node (ID:" + (String)fromAddress + ") attempted communication.");
                        }       
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

    nodeID = 1; // hardcoded for receiver node
    setupRadio();

    // create the nodeStatus for our receiver
    NodeStatus receiverStatus = {};
    receiverStatus.nodeId = nodeID;
    receiverStatus.messageId = 0;
    receiverStatus.batteryVoltage = getBatteryVoltage();
    strlcpy(receiverStatus.nodeName, "receiver", sizeof(receiverStatus.nodeName));

    // set the receiver's mac
    WiFi.macAddress(receiverStatus.nodeMACAddress);

    // append the receiver status; ignore the return because this is the first append.
    (void)appendToNetwork(receiverStatus);

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
        5000,
        NULL,
        1,
        NULL,
        0
    );
}

void loop() {
    server.handleClient();
    if(millis() - lastScreenUpdate > 5000){ // if it has been 5 sec since hte last screen update
        updateScreen();
        lastScreenUpdate = millis();
    }
}

#endif