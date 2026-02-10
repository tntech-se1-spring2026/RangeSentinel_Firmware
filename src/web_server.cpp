#include "web_server.h"

void startWebServer() {
    AsyncWebServer server(80);

    startBackend(&server);
    startFileServer(&server);
    startAPI(&server);

    server.begin();
}

void startFileServer(AsyncWebServer *server) {
    // serves up files in www folder as requests come in
    server->serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");
}

void startBackend(AsyncWebServer *server) {
    // Basic sanity route
    server->on("/web/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "pong");
        response->addHeader("Server", "ESP Async Web Server");
        request->send(response);
    });
}

void startAPI(AsyncWebServer *server) {
    // API for inter-node comms here?
    //
    // Not sure if HTTP over LoRa is
    // what you're going for though...
}
