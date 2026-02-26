#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "database_manager.h"
#include <LittleFS.h>
#include <WebServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>

#define HTTP_PORT 80 // standard HTTP port

extern AsyncWebSocket ws;

/// @brief Initialize the web interface
/// @param server Pointer to the AsyncWebServer instance
void startWebServer(AsyncWebServer *server);

/// @brief Allows files in littleFS's /www to be arbitrarily served up
/// @param server Pointer to the AsyncWebServer instance
void startFileServer(AsyncWebServer *server);

/// @brief Set up the core system logic
/// @param server Pointer to the AsyncWebServer instance
void startBackend(AsyncWebServer *server);

/// @brief ...
/// @param server ...
void startAPI(AsyncWebServer *server);

#endif
