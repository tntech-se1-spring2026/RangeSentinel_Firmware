#include "hardware_manager.h"
#include "shared_types.h"
#include "database_manager.h"

#pragma region VARIABLES
// --- RADIO COMM ---
RH_RF95 rf95(RFM95_CS, RFM95_INT); // radio driver
RHMesh* manager;
double messagesSent = 0;

// --- SCREEN ---
#ifdef IS_LILYGO_T3
    Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
    int brightness = 255;
#endif

// --- DOOR SENSOR ---
int prevRSState = -1; // Set to -1 so it prints the initial state once
int currentRSState = 0;
#pragma endregion

#pragma region FUNCTIONS
// --- SCREEN ---
void setupScreen(){
    Wire.begin(OLED_SDA, OLED_SCL); 
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("OLED failed to initialize"));
        while(true);
    }

    // 0 is darkest, 255 is brightest
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(brightness);

    // put starting message
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // --- Starting Message (Redesigned & Centered) ---
    
    // Line 1: RANGE SENTINEL (Size 1)
    // 14 chars * 6px = 84px. (128 - 84) / 2 = 22
    display.setTextSize(1);
    display.setCursor(22, 10);
    display.println(F("RANGE SENTINEL"));

    // Line 2: VIEWER (Size 2)
    // 6 chars * 12px = 72px. (128 - 72) / 2 = 28
    display.setTextSize(2);
    display.setCursor(28, 25);
    display.println(F("VIEWER"));

    // Decorative Divider (aesthetic touch)
    display.drawFastHLine(34, 45, 60, SSD1306_WHITE);

    // Line 3: System Booting (Size 1)
    // 14 chars * 6px = 84px. (128 - 84) / 2 = 22
    display.setTextSize(1);
    display.setCursor(22, 52);
    display.print(F("SYSTEM BOOTING"));

    display.display();
    
    analogReadResolution(12); // ESP32 ADC is 12-bit (0-4095)
}

void updateScreen() {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // --- 1. HEADER BAR ---
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("RANGE SENTINEL");

    // Client Icon + Count (Phones connected to AP)
    int clientCount = WiFi.softAPgetStationNum();
    display.drawCircle(115, 2, 2, SSD1306_WHITE); 
    display.drawRect(112, 5, 7, 2, SSD1306_WHITE); 
    display.setCursor(121, 0); 
    display.printf("%d", clientCount);

    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

    // --- 2. PRIMARY DATA (Centered) ---
    display.setTextSize(1);
    display.setCursor(22, 18);
    display.print("Internal Power");

    int pct = getBatteryPercentage();
    int xPos = (pct >= 100) ? 28 : (pct < 10) ? 46 : 37; // Handles 1, 2, or 3 digits
    
    display.setTextSize(3);
    display.setCursor(xPos, 30);
    display.printf("%d%%", pct);

    // --- 3. FOOTER DATA (LoRa Mesh Status) ---
    display.drawLine(0, 54, 127, 54, SSD1306_WHITE);
    
    // Mesh Icon (3 dots connected by lines)
    display.drawCircle(2, 60, 1, SSD1306_WHITE); // Dot 1
    display.drawCircle(8, 60, 1, SSD1306_WHITE); // Dot 2
    display.drawCircle(5, 57, 1, SSD1306_WHITE); // Dot 3
    display.drawLine(2, 60, 5, 57, SSD1306_WHITE);
    display.drawLine(8, 60, 5, 57, SSD1306_WHITE);

    display.setTextSize(1);
    display.setCursor(14, 57);
    display.printf("NODES: %u", numNodesInNetwork);
    //Serial.println("update screen: numNodesInNetwork: " + String(numNodesInNetwork));
    
    // Voltage: Bottom Right
    display.setCursor(98, 57);
    display.printf("%.2fV", getBatteryVoltage());

    display.display();
}

// --- RADIO ---
void setupRadio(uint8_t nodeID){
    messagesSent = 0;

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_SS);

    // Manual reset of the LoRa radio to ensure a clean state
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    pinMode(RFM95_INT, INPUT);

    if(!rf95.init()){
        Serial.println("RF95 Driver init failed!");
    }

    // set Radio frequency
    if (!rf95.setFrequency(915.0)){
        Serial.println("setFrequency failed");
    }

    // false = don't use RFO pin, use PA_BOOST (standard for SX1276)
    rf95.setTxPower(RADIO_POWER, false);

    // This helps the driver align its timing with the hardware
    if (!rf95.setModemConfig(RH_RF95::Bw125Cr45Sf128)) {
        Serial.println("Invalid modem config!");
    }

    manager = new RHMesh(rf95, nodeID);

    if (!manager->init()) {
        Serial.println("Mesh init failed!");
        while(true);
    }
}

void receiverListen(void* pvParameters){
    while(true){
        uint8_t incoming[RH_MESH_MAX_MESSAGE_LEN];
        uint8_t len = sizeof(incoming);
        uint8_t fromAddress;
        
        // recvfromAck returns true if addressed for us, if not returns false & forwards to appropriate node.
        if (manager->recvfromAck(incoming, &len, &fromAddress)){ // true if a message is heard
            Serial.println("Message received from " + String(fromAddress));

            // translate incoming into a readable mesh packet
            MeshPacket incomingPacket;
            if(!deserializePacket(incoming, len, incomingPacket)){
                Serial.println("Packet Failed to deserialize");
                continue;
            }

            // --- CASE 1: UNASSIGNED NODE (ID 254) ---
            if(fromAddress == UNASSIGNED_ID){
                Reading* req2Asn = getReadingOfType(incomingPacket.readings, REQUEST_TO_ASSIGN);
                
                if(req2Asn){ // Check if it is sending a request to be assigned (if it wasn't requesting assignment, then this will be false)
                    int nodeIdx = findNodeIndexByMAC(req2Asn->payload.asMAC); 
                    
                    if(nodeIdx != -1){
                        // Node is known, send back its existing ID
                        Serial.printf("Known MAC found. Re-assigning ID: %d\n", networkDatabase[nodeIdx].nodeID);
                        sendAssignNodeID(networkDatabase[nodeIdx].nodeID, req2Asn->payload.asMAC); 
                    }else{
                        // TODO: eventually we will allow the user to accept the node to be added, for now we will just add it
                        // Add new node to database & assign it's ID
                        uint8_t newID = addNodeToNetworkDatabase(incomingPacket);
                        sendAssignNodeID(newID, req2Asn->payload.asMAC); // Send to broadcast or MAC-specific logic
                    }
                } 
            }else{
                // --- CASE 2: ASSIGNED NODE ---
                if(fromAddress > numNodesInNetwork){
                    // This happens if the Viewer rebooted and lost its memory
                    // TODO: Tell the node to re-identify or just add it on the fly
                    
                }else{
                    // Update the database with the latest readings
                    updateDatabase(incomingPacket, fromAddress);
                }
            }
        }
        vTaskDelay(1 / portTICK_PERIOD_MS); // FEED THE WATCHDOG: This 1ms pause is mandatory on Core 0 otherwise we will be at 100% usage and 
    }
}
 
void sensorListen(){
    uint8_t incoming[RH_MESH_MAX_MESSAGE_LEN]; // buffer to hold the raw bytes of any incoming message.
    uint8_t len = sizeof(incoming);
    uint8_t fromAddress;

    // recvfromAck returns true if addressed for us, if not returns false & forwards to appropriate node.
    if (manager->recvfromAck(incoming, &len, &fromAddress)){
        Serial.println("Message received from " + String(fromAddress));

        // translate incoming into a readable mesh packet
        MeshPacket incomingPacket;
        if(!deserializePacket(incoming, len, incomingPacket)){
            Serial.println("Packet Failed to deserialize");
        }

        // check if we still need assigned
        if(nodeID == UNASSIGNED_ID){
            // ensure sender is correct
            if(fromAddress == VIEWER_ID){
                // grab values from transmission
                Reading* assignedMAC = getReadingOfType(incomingPacket.readings, ASSIGNMENT_MAC);
                Reading* assignedID = getReadingOfType(incomingPacket.readings, ASSIGNMENT_ID);

                // ensure both readings exist
                if(assignedID && assignedMAC){
                    // grab mac
                    uint8_t sensorMAC[6];
                    esp_read_mac(sensorMAC, ESP_MAC_WIFI_STA);

                    // check if our mac matches
                    if(memcmp(assignedMAC->payload.asMAC, sensorMAC, 6) == 0){
                        // assign your new ID
                        nodeID = assignedID->payload.asByte;
                        rf95.setThisAddress(nodeID);
                        manager->setThisAddress(nodeID);
                        
                        Serial.println("Set my ID to: " + String(nodeID));
                    }
                }
            }
        }else{
            // normal listen behavior when assigned
        }
    }
}

void sendAssignNodeID(uint8_t desiredID, uint8_t* nodeMAC){
    // Create readings
    Reading assignmentIDData;
    assignmentIDData.type = ASSIGNMENT_ID;
    assignmentIDData.payload.asByte = desiredID;

    Reading assignmentMACData;
    assignmentMACData.type = ASSIGNMENT_MAC;
    memcpy(assignmentMACData.payload.asMAC, nodeMAC, 6);
    
    // Create Packet
    MeshPacket assignmentPacket;
    assignmentPacket.messageId = messagesSent++;
    assignmentPacket.readingCount = 2;
    assignmentPacket.readings[0] = assignmentIDData;
    assignmentPacket.readings[1] = assignmentMACData;

    // Serialize Packet
    uint8_t rawMessage[RH_MESH_MAX_MESSAGE_LEN];
    uint8_t numBytes = serializePacket(assignmentPacket, rawMessage, RH_MESH_MAX_MESSAGE_LEN);

    Serial.printf("Broadcasting assignment: ID %d to MAC\n", desiredID);

    // Sends the assignment 3 times as a broadcast. The sensor will know to assign itself based on the MAC.
    for(int i = 0; i < 3; i++){
        bool error = manager->sendtoWait(rawMessage, numBytes, RH_BROADCAST_ADDRESS);
        delay(200);
        if(error){
            Serial.println("assign send failed!");
        }
    }
}

void sendRequestAssignment(){
    // Create readings
    Reading request;
    request.type = REQUEST_TO_ASSIGN;
    uint8_t sensorMAC[6];
    esp_read_mac(sensorMAC, ESP_MAC_WIFI_STA);
    memcpy(request.payload.asMAC, sensorMAC, 6);

    // Create Packet
    MeshPacket requestPacket;
    requestPacket.messageId = messagesSent++;
    requestPacket.readingCount = 1;
    requestPacket.readings[0] = request;

    // Serialize Packet
    uint8_t rawMessage[RH_MESH_MAX_MESSAGE_LEN];
    uint8_t numBytes = serializePacket(requestPacket, rawMessage, RH_MESH_MAX_MESSAGE_LEN);
    if (numBytes == 0) {
        Serial.println("Error: Serialization returned 0!");
        return;
    }

    // send packet
    bool error = manager->sendtoWait(rawMessage, numBytes, RH_BROADCAST_ADDRESS);

    // Check for error
    if(error != RH_ROUTER_ERROR_NONE) {
        Serial.printf("Mesh sendToWait broadcast failed! Error code: %d\n", error);
    }
}

bool sendHeartBeat(){
    Serial.println("Sending heartbeat");

    // create reading
    Reading batteryReading;
    batteryReading.type = BATTERY_SENSOR;
    float voltage = getBatteryVoltage();
    batteryReading.payload.asFloat = voltage;

    // create packet
    MeshPacket heartBeatPacket;
    heartBeatPacket.messageId = messagesSent++;
    heartBeatPacket.readingCount = 1;
    heartBeatPacket.readings[0] = batteryReading;

    // serialize
    uint8_t rawMessage[RH_MESH_MAX_MESSAGE_LEN];
    uint8_t numBytesInSerializedPacket = serializePacket(heartBeatPacket, rawMessage, RH_MESH_MAX_MESSAGE_LEN);

    // send packet
    uint8_t error = manager->sendtoWait(rawMessage, numBytesInSerializedPacket, VIEWER_ID); // send to viewer nodeID

    // Check for error
    if (error != RH_ROUTER_ERROR_NONE) {
        Serial.println("Heartbeat message failed. Sending again");
        return true;
    }else{
        Serial.println("Heartbeat message succeeded.");
    }

    return false;
}

bool sendReedSwitchMessage(int switchState){
    // create readings
    Reading batteryReading;
    batteryReading.type = BATTERY_SENSOR;
    float voltage = getBatteryVoltage();
    batteryReading.payload.asFloat = voltage;
    
    Reading doorReading;
    doorReading.type = DOOR_SENSOR;
    doorReading.payload.asBool = switchState;

    // create packet
    MeshPacket doorPacket;
    doorPacket.messageId = messagesSent++;
    doorPacket.readingCount = 1;
    doorPacket.readings[0] = batteryReading;
    doorPacket.readings[0] = doorReading;

    // serialize
    uint8_t rawMessage[RH_MESH_MAX_MESSAGE_LEN];
    uint8_t numBytesInSerializedPacket = serializePacket(doorPacket, rawMessage, RH_MESH_MAX_MESSAGE_LEN);

    // send
    uint8_t error = manager->sendtoWait(rawMessage, numBytesInSerializedPacket, VIEWER_ID);

    // Check for error
    if (error != RH_ROUTER_ERROR_NONE) {
        Serial.println("Door message failed. Sending again");
        return true;
    }else{
        Serial.println("Door message succeeded.");
    }

    return false;
}

// --- BATTERY ---
float getBatteryVoltage(){
    #define ADC_PIN 35 // Battery is hardwired to GPIO 35 on our T3_v1.6.1 board

    uint16_t raw = analogRead(ADC_PIN); // Read Raw ADC value

    // Convert and return voltage
    // The board uses a divider that halves the voltage, so we multiply by 2.
    // 3.3V / 4095 units * 2 (divider) * 1.1 (typical ESP32 calibration)
    return ((raw / 4095.0) * 3.3 * 2.0 * 1.1);
}

// TODO: Make this account for non linear discharging
int getBatteryPercentage(){
    // Li-ion range: 4.2V (100%) down to 3.2V (0%)
    int percentage = (int)((getBatteryVoltage() - 3.2) / (4.2 - 3.2) * 100);
    return(constrain(percentage, 0, 100));
}

// TODO: Make this account for non linear discharging
int getBatteryPercentageFromV(float voltage){
    // Li-ion range: 4.2V (100%) down to 3.2V (0%)
    int percentage = (int)((voltage - 3.2) / (4.2 - 3.2) * 100);
    return(constrain(percentage, 0, 100));
}

void reedSwitchLogic(){
    // grab door state
    currentRSState = digitalRead(RS_PIN);

    // Only do something if the state changed
    if (currentRSState != prevRSState) {
        if(currentRSState != LOW){
            // Magnet is NEAR (Completes the circuit to GND)
            Serial.println("Status: DOOR CLOSED");
        }else{
            // Magnet is GONE (Internal pull-up makes it HIGH)
            Serial.println("Status: DOOR OPEN!");
        }

        // send message until it works
        while(sendReedSwitchMessage(currentRSState));

        // Update the memory
        prevRSState = currentRSState;
    }
}
#pragma endregion