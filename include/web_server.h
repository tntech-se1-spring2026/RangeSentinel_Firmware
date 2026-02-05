#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>

// officially declared in main.cpp
extern WebServer server;

void setupWebServer(String (*getStatusJson)(), String (*getLogJson)());

#endif
