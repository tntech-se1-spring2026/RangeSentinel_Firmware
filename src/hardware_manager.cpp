#include "hardware_manager.h"
#include "shared_types.h"
#include "database_manager.h"

// --- RADIO COMM ---
RH_RF95 rf95(RFM95_CS, RFM95_INT); // radio driver
RHMesh* manager; // obj to manage mesh comm routing.

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int prevRSState = -1; // Set to -1 so it prints the initial state once
int brightness = 255;

RH_RF95 getRadio(){
    return rf95;
}

// TODO: Finish this function
void assignNewNodeID(const char* macStr){
    // Convert string "AA:BB..." to raw bytes
    uint8_t tempMac[6];
    sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
           &tempMac[0], &tempMac[1], &tempMac[2], 
           &tempMac[3], &tempMac[4], &tempMac[5]);

    // Check if MAC exists in networkDatabase
    uint8_t assignedID = 0;
    for(NodeStatus &node : networkDatabase){
        if(memcmp(node.nodeMACAddress, tempMac, 6) == 0){
            assignedID = node.nodeId;
            break;
        }
    }

    // If not, find next ID and appendToNetwork

    // Send response back to Node 0 via manager->sendtoWait
}

void setupRadio(uint8_t nodeID){
    // Manual reset of the LoRa radio to ensure a clean state
    Serial.println("Reseting radio");
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, LOW);  // Pull low to reset
    delay(10); 
    digitalWrite(RFM95_RST, HIGH); // Pull high to operate
    delay(10);

    // Initialize the Mesh Manager & LoRa driver (rf95)
    manager = new RHMesh(rf95, nodeID);
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
}

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
    
    // Voltage: Bottom Right
    display.setCursor(98, 57);
    display.printf("%.2fV", getBatteryVoltage());

    display.display();
}

void receiverListen(void* pvParameters){
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
                // parse incoming json
                JsonDocument incomingDoc;
                DeserializationError err = deserializeJson(incomingDoc, incoming);

                if(err){ // if failed
                    Serial.print("JSON parse failed: ");
                    Serial.println(err.c_str());
                
                }else{
                    if(fromAddress == 0){ // if this is a new/unassigned node
                        const char* incomingMac = incomingDoc["mac"] | ""; 
                        if (strlen(incomingMac) > 0) {
                            assignNewNodeID(incomingMac);
                        }
                    }else{ // already assigned node
                        bool found = false;
                        for(NodeStatus &node : networkDatabase){ // check each node in db to see if matches
                            if(node.nodeId == fromAddress){ // if node matches, update our db
                                // grab mac as string
                                const char* incomingMACString = incomingDoc["mac"] | ""; 
                                
                                // convert string to raw bytes
                                uint8_t incomingMACRaw[6];
                                sscanf(incomingMACString, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
                                    &incomingMACRaw[0], &incomingMACRaw[1], &incomingMACRaw[2], 
                                    &incomingMACRaw[3], &incomingMACRaw[4], &incomingMACRaw[5]);

                                // check if matches
                                if(memcmp(node.nodeMACAddress, incomingMACRaw, 6) == 0){
                                    found = true;
                                    // update db
                                    node.messageId = incomingDoc["mId"].as<long>();
                                    node.batteryVoltage = incomingDoc["batt"].as<float>();
                                    node.motionDetected = incomingDoc["motion"].as<bool>();
                                    node.doorOpen = incomingDoc["door"].as<bool>();
                                    strlcpy(node.nodeName, incomingDoc["name"].as<const char*>(), sizeof(node.nodeName));
                                    // exit loop
                                    break;
                                }else{
                                    // TODO: Decide how to handle an incoming message with an existing ID, but without a matching MAC
                                }
                            }
                        }
                        if(!found){ // if our sending node wasn't found in our db
                            Serial.println("Warning: Unrecognized node (ID:" + (String)fromAddress + ") attempted communication.");
                        }       
                    }
                }
            }
            xSemaphoreGive(meshMutex); // UNLOCK
        }
        // FEED THE WATCHDOG: This 1ms pause is mandatory on Core 0
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void sensorListen(){ // Must be called constantly to process incoming packets
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

void reedSwitchLogic(){
    int currentState = digitalRead(RS_PIN);

    // Only do something if the state changed
    if (currentState != prevRSState) {
        uint8_t data[1] = {(uint8_t)currentState};
        rf95.send(data, sizeof(data));
        rf95.waitPacketSent();
    
        Serial.print("State changed! New state: ");
        Serial.println(currentState == HIGH ? "OPEN" : "CLOSED");

        // Update the memory
        prevRSState = currentState;
    }
}