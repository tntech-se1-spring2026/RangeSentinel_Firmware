#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H

#include "pins.h" // has all our physical pins

#include <Wire.h> // for low level comm w/screen
#include <Adafruit_GFX.h> // graphics for screen
#include <Adafruit_SSD1306.h> // driver chip for screen

#include <SPI.h> // translator for SX1276 LoRa chip
#include <RH_RF95.h> // The physical layer driver (SX1276)
#include <RHMesh.h> // The network layer manager (Routing/Mesh)

// --- VARIABLES ---
extern int prevRSState;

// --- FUNCTIONS ---
void assignNewNodeID(const char* macStr);
void setupRadio(uint8_t nodeID);
void setupScreen();
float getBatteryVoltage();
int getBatteryPercentage();
void updateScreen();
void receiverListen(void* pvParameters);
void sensorListen();
void reedSwitchLogic();

#endif