#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <LittleFS.h>
#include "shared_types.h"

void setupWebServer(String (*getStatusJson)());

#endif
