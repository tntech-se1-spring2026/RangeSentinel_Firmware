#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

// officially declared in main.cpp

// // void setupWebServer(String (*getStatusJson)(), String (*getLogJson)());
void startWebServer();
void startFileServer(AsyncWebServer *server);
void startBackend(AsyncWebServer *server);
void startAPI(AsyncWebServer *server);

#endif
