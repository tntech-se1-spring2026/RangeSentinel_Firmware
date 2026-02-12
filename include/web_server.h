#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

void startWebServer(AsyncWebServer *server);

void startFileServer(AsyncWebServer *server);
void startBackend(AsyncWebServer *server);
void startAPI(AsyncWebServer *server);

#endif
