#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

extern WebServer server;
extern String WiFiPassword;
void startWebServer(AsyncWebServer *server);

void startFileServer(AsyncWebServer *server);
void startBackend(AsyncWebServer *server);
void startAPI(AsyncWebServer *server);

#endif