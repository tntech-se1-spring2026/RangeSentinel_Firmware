#ifndef HARDWARE_MANAGER_H
#define HARDWARE_MANAGER_H

#include "pins.h" // has all our physical pins
#include "web_server.h"

#include <Wire.h> // for low level comm w/screen
#include <WiFi.h>
#include <Adafruit_GFX.h> // graphics for screen
#include <Adafruit_SSD1306.h> // driver chip for screen

#include <SPI.h> // translator for SX1276 LoRa chip
#include <RH_RF95.h> // The physical layer driver (SX1276)
#include <RHMesh.h> // The network layer manager (Routing/Mesh)

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// --- VARIABLES ---
extern int prevRSState;
extern int brightness;
extern RH_RF95 rf95;
// RHMesh* manager;
extern Adafruit_SSD1306 display;

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
void updateLocalDisplay(bool doorOpen, float voltage);

#endif