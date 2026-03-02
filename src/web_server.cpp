#ifdef NODE_TYPE_VIEWER

#include "web_server.h"

#define HTTP_URL "http://range-sentinel.com"
#define URL "range-sentinel.com"

AsyncWebSocket ws("/ws");

AsyncMiddlewareFunction ensureURL([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    Serial.println(request->host());
    if (request->host() == URL) {
        next();
    } else {
        return request->redirect(HTTP_URL);
    }
});

void startWebServer(AsyncWebServer *server) {
    // register WebSocket handler
    server->addHandler(&ws);

    // clean up memory from disconnected clients occasionally
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_DISCONNECT) {
            Serial.printf("WS: Client %u disconnected\n", client->id());
        }
    });

    server->addMiddleware(&ensureURL);

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
        // 1. Create response. 'true' means the root is an array [].
        // Increase the buffer size (e.g., 4096) if your database is large!
        AsyncJsonResponse *response = new AsyncJsonResponse(true);
        
        // 2. Get the actual live root of the response
        JsonVariant root = response->getRoot();

        // 3. Deserialize the string directly into the RESPONSE object
        DeserializationError error = deserializeJson(root, getDatabaseAsJson());

        Serial.println(getDatabaseAsJson());

        if (error) {
            Serial.print("JSON Error: ");
            Serial.println(error.f_str());
            request->send(500, "text/plain", "JSON Parse Error");
            return;
        }

        // 4. Modify the data in place (Battery Logic)
        // 'root' now contains your array. We just loop through it.
        JsonArray nodes = root.as<JsonArray>();
        for (JsonObject node : nodes) {
            JsonArray sensors = node["sensors"].as<JsonArray>();
            for (JsonObject sensor : sensors) {
                if (sensor["type"] == "batt" || sensor["type"] == "battery") {
                    float voltage = sensor["val"];
                    sensor["val"] = getBatteryPercentageFromV(voltage);
                }
            }
        }

        // TODO: add last seen logic

        // 5. Finalize and send
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
}

void startAPI(AsyncWebServer *server) {
    // API for inter-node comms here?
    //
    // Not sure if HTTP over LoRa is
    // what you're going for though...
}

#endif
