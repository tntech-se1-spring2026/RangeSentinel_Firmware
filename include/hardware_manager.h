#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H

#include "pins.h" // has all our physical pins

#include <WiFi.h>

#include <Wire.h> // for low level comm w/screen
#include <Adafruit_GFX.h> // graphics for screen
#include <Adafruit_SSD1306.h> // driver chip for screen

#include <SPI.h> // translator for SX1276 LoRa chip
#include <RH_RF95.h> // The physical layer driver (SX1276)
#include <RHMesh.h> // The network layer manager (Routing/Mesh)

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define UNASSIGNED_ID   254
#define VIEWER_ID       1  
#define RADIO_POWER     10 // TX Power: 5 to 23 dBm. 23 is max power. 

typedef enum{
    CAMERA,
    REED_SWITCH
} SensorType;

#pragma region VARIABLES
// radio
extern RH_RF95 rf95;
extern RHMesh* manager;
extern double messagesSent;
extern uint8_t nodeID;
extern unsigned long lastHeartBeat;
// screen
extern Adafruit_SSD1306 display;
extern int brightness;
// door sensor
extern int prevRSState;
extern int currentRSState;
// WiFi
extern String WiFiPassword;
#pragma endregion

#pragma region FUNCTIONS
// --- SCREEN ---
/// @brief turns on the screen & posts the booting page
void setupScreen();

/// @brief posts the main page with updated data
void updateScreen();

// --- RADIO ---
/// @brief resets the radio, inits the manager, sets the freq, & sets radio power (23dBm current)
/// @param nodeID sets the nodeID for the manager
void setupRadio(uint8_t nodeID);

/// @brief listens for and handles incoming packets from the network
/// @param pvParameters parameters for running on its own core.
void receiverListen(void* pvParameters);

/// @brief listens for and handles incoming packets from the network. Must be called constantly to process incoming packets
void sensorListen();

/// @brief This function sends the node assignment packet
/// @param desiredID The nodeID that needs to be assigned to the node
/// @param nodeMAC The MAC of the node that needs to be assigned
void sendAssignNodeID(uint8_t desiredID, uint8_t* nodeMAC);

/// @brief This function is called by the sensor node when it hasn't been assigned a nodeID in the network. It runs this every minute until it is assigned
void sendRequestAssignment();

/// @brief This function sends a very basic message just saying "I'm alive!"
/// @returns returns true if message failed
bool sendHeartBeat();

/// @brief This functions ends a message containing the door switch status
/// @param switchState the current state of the switch
/// @returns returns true if message failed
bool sendReedSwitchMessage(int switchState);

// --- BATTERY ---
/// @brief calculates the battery's current voltage
/// @returns returns the battery's current voltage
float getBatteryVoltage();

/// @brief calculates the battery's current percentage from the voltage 
/// @returns returns the battery's current percentage
int getBatteryPercentage();

// --- REED SWITCH (DOOR SENSOR) ---
/// @brief NOT FINISHED; CURRENTLY JUST PRINTS OUT WHEN OPEN/CLOSED
void reedSwitchLogic();
#pragma endregion
#endif