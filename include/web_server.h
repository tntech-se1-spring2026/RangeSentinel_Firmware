#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

extern WebServer server;
extern String WiFiPassword;

void setupWebServer(String (*getStatusJson)(), String (*getLogJson)());

#endif