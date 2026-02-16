#ifdef NODE_TYPE_VIEWER

#include "web_server.h"

AsyncWebSocket ws("/ws");

void startWebServer(AsyncWebServer *server) {
    // register WebSocket handler
    server->addHandler(&ws);

    // clean up memory from disconnected clients occasionally
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_DISCONNECT) {
            Serial.printf("WS: Client %u disconnected\n", client->id());
        }
    });


    startBackend(server);
    startFileServer(server);
    startAPI(server);

    server->begin();
}

void startFileServer(AsyncWebServer *server) {
    // serves up files in www folder as requests come in
    server->serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");
}

void startBackend(AsyncWebServer *server) {
    // Basic sanity route
    server->on("/web/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "pong");
        Serial.println("Ping request made");
        response->addHeader("Server", "ESP Async Web Server");
        request->send(response);
    });

    // exports node db
    server->on("/web/nodes", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncJsonResponse *response = new AsyncJsonResponse();

        JsonDocument doc;
        deserializeJson(doc, getDatabaseAsJson());

        response->getRoot() = doc.to<JsonObject>();
        response->setLength();

        request->send(response);
    });
}

void startAPI(AsyncWebServer *server) {
    // API for inter-node comms here?
    //
    // Not sure if HTTP over LoRa is
    // what you're going for though...
}

#endif