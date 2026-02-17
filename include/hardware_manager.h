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

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#pragma region VARIABLES
// radio
extern RH_RF95 rf95;
extern RHMesh* manager;
// screen
extern Adafruit_SSD1306 display;
extern int brightness;
// door sensor
extern int prevRSState;
// WiFi
extern String WiFiPassword;
#pragma endregion

#pragma region FUNCTIONS
// --- SCREEN ---
/// @brief
void setupScreen();

/// @brief 
void updateScreen();

// --- RADIO ---
/// @brief 
/// @param nodeID 
void setupRadio(uint8_t nodeID);

/// @brief listens for and handles incoming packets from the network
/// @param pvParameters parameters for running on its own core.
void receiverListen(void* pvParameters);

/// @brief
void sensorListen();

// --- BATTERY ---
/// @brief
/// @returns
float getBatteryVoltage();

/// @brief
/// @returns
int getBatteryPercentage();

// --- REED SWITCH (DOOR SENSOR) ---
/// @brief
void reedSwitchLogic();
#pragma endregion
#endif