#include "hardware_manager.h"
#include "database_manager.h"

// holds the time of the last screen update; used to check if we need to update screen again
unsigned long lastScreenUpdate = 0;
const long fiveSecInterval      = 5000;
unsigned long currentMS         = 0;


#ifdef NODE_TYPE_SENSOR
bool lastDoorState              = -1;
unsigned long lastPollMS        = 0;
unsigned long lastHeartbeatMS   = 0;
bool currentDoorState           = false;

uint32_t msgCount               = 0;


void sendJsonStatus(bool doorOpen) {
    StaticJsonDocument<256> doc;
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);

    doc["nodeId"] = 2;
    doc["msgId"] = msgCount++;
    doc["v"] = getBatteryVoltage();
    doc["door"] = doorOpen;
    
    // Convert MAC to readable string
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    doc["mac"] = macStr;

    // Serialize JSON to buffer
    char buffer[256];
    serializeJson(doc, buffer);

    // Send via LoRa
    rf95.send((uint8_t*)buffer, strlen(buffer));
    rf95.waitPacketSent();
    Serial.println("Sent: " + String(buffer));
}

void setup() {
    pinMode(RS_PIN, INPUT_PULLUP); // set the reed switch's pin's mode
    Serial.begin(115200);

    if (!rf95.init()) Serial.println("LoRa radio init failed");
    rf95.setFrequency(915.0); // Match your region
    rf95.setTxPower(23, false);

    setupScreen();
}


void loop() {
    currentMS = millis();
    
    // 1. POLL: Check the physical pin every 50ms
    if(currentMS - lastPollMS > 50){
        currentDoorState = digitalRead(4);
        
        if (currentDoorState != lastDoorState) {
            Serial.println("Door change: " + String(currentDoorState));
            sendJsonStatus(currentDoorState); 
            updateLocalDisplay(currentDoorState, getBatteryVoltage());
            
            lastDoorState = currentDoorState;
            lastHeartbeatMS = currentMS; // Reset heartbeat because we just sent data
        }
        lastPollMS = currentMS; // Update the polling clock
    }

    // 2. HEARTBEAT: If it's been 15s since the LAST LoRa transmission
    if(currentMS - lastHeartbeatMS > 15000){
        Serial.println("Routine heartbeat send...");
        sendJsonStatus(currentDoorState);
        updateLocalDisplay(currentDoorState, getBatteryVoltage());
        lastHeartbeatMS = currentMS; // Update the heartbeat clock
    }
}
#endif

// --- viewing node ---
#ifdef NODE_TYPE_VIEWER

// standard HTTP port
WebServer server(80);

unsigned long lastScreenMS      = 0;
unsigned long lastDBMS          = 0;
const long fiveMinInterval      = 300000;

String WiFiPassword             = "password";



void setup(){
    Serial.begin(115200);

    setupScreen();

    // initialize the mutex to protect db shared btwn cores
    meshMutex = xSemaphoreCreateMutex();

    // setup radio
    if (!rf95.init()) Serial.println("LoRa init failed");
    rf95.setFrequency(915.0);

    // create the nodeStatus for our receiver
    NodeStatus receiverStatus = {};
    receiverStatus.nodeId = 1;
    receiverStatus.messageId = 0;
    receiverStatus.batteryVoltage = getBatteryVoltage();
    strlcpy(receiverStatus.nodeName, "receiver", sizeof(receiverStatus.nodeName));
    // MAC
    uint8_t recRawMAC[6];
    esp_read_mac(recRawMAC, ESP_MAC_WIFI_STA);
    char recStrMAC[18];
    snprintf(recStrMAC, sizeof(recStrMAC), "%02X:%02X:%02X:%02X:%02X:%02X", 
             recRawMAC[0], recRawMAC[1], recRawMAC[2], recRawMAC[3], recRawMAC[4], recRawMAC[5]);
    receiverStatus.nodeMACAddress = recStrMAC;
    // append the receiver status; ignore the return because this is the first append.
    (void)appendToNetwork(receiverStatus);

    // create the nodeStatus for our sensor
    NodeStatus sensorStatus = {};
    sensorStatus.nodeId = 2;
    sensorStatus.messageId = 0;
    sensorStatus.batteryVoltage = 3.7f;
    strlcpy(sensorStatus.nodeName, "Door 1", sizeof(sensorStatus.nodeName));
    // MAC
    receiverStatus.nodeMACAddress = "";
    // append the receiver status; ignore the return because this is the first append.
    (void)appendToNetwork(sensorStatus);

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

    setupWebServer(getDatabaseAsJson, getEventLogAsJson);

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

    // handle webserver client
    server.handleClient();
    
    // update OLED screen
    if(currentMS - lastScreenUpdate > fiveSecInterval){ // if it has been 5 sec since the last screen update
        updateScreen();
        lastScreenUpdate = millis();
    }

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