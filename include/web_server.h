#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "database_manager.h"
#include <LittleFS.h>
#include <WebServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>

#define HTTP_PORT 80 // standard HTTP port

/// @brief 
/// @param server 
void startWebServer(AsyncWebServer *server);

/// @brief 
/// @param server 
void startFileServer(AsyncWebServer *server);

/// @brief 
/// @param server 
void startBackend(AsyncWebServer *server);

/// @brief
/// @param server
void startAPI(AsyncWebServer *server);

#endif