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
        AsyncJsonResponse *response = new AsyncJsonResponse(true);  // true to expect array

        JsonDocument doc;
        deserializeJson(doc, getDatabaseAsJson());

        response->getRoot() = doc.to<JsonArray>();
        response->setLength();

        request->send(response);
    });

    // acknowledge alerts
    server->on("/web/ack", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("id")) {
            String idStr = request->getParam("id")->value();
            idStr.trim();
            // check ID is not empty
            if (idStr.length() == 0) {
                request->send(400, "text/plain", "Error: ID parameter is empty");
                return;
            }
            // check each character is a digit
            for (int i = 0; i < idStr.length(); i++) {
                if (!isDigit(idStr.charAt(i))) {
                request->send(400, "text/plain", "Error: ID must be an integer");
                return;
                }
            }

            // convert to long first to prevent overflow
            long parsedId = idStr.toInt(); 
            if (parsedId > 255) {
                request->send(400, "text/plain", "Error: ID out of range (must be 0-255).");
                return;
            }

            uint8_t id = (uint8_t)parsedId;

            if (clearAlertLatch(id)) {
                request->send(200, "text/plain", "Ok: Latch cleared");
            }
            else {
                request->send(404, "text/plain", "Error: Invalid Node ID");
            }
        }
        else {
            request->send(400, "text/plain", "Error: Missing id parameter");
        }
    });

    bool simulationActive = false;  // simulation is off by default
    server->on("web/simulate", HTTP_POST, [&simulationActive](AsyncWebServerRequest *request) {
        simulationActive = !simulationActive;
        request->send(200, "text/plain", simulationActive ? "Simulation ON": "Simulation Off");
    });
}

void startAPI(AsyncWebServer *server) {
    // API for inter-node comms here?
    //
    // Not sure if HTTP over LoRa is
    // what you're going for though...
}

#endif