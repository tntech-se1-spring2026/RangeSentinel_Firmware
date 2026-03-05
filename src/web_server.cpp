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
        // handles battery percentage and simplified node json for website
        request->send(200, "application/json", getDatabaseForWeb());
    });

    // export alerts
    server->on("/web/alerts", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "applications/json", getActiveAlertsAsJson());
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

    // event history
    server->on("/web/history", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "applications/json", getEventLogAsJson());
    });

    // renaming
    server->on("/web/rename", HTTP_POST, [](AsyncWebServerRequest *request) {
        // make sure target nodeID and new name string are present in request
        if (request->hasParam("id") && request->hasParam("name")) {
            String idStr = request->getParam("id")->value();
            String newName = request->getParam("name")->value();
            
            // validate id is an integer
            for (int i = 0; i < idStr.length(); i++) {
                if (!isDigit(idStr.charAt(i))) {
                    request->send(400, "text/plain", "Error: ID must be a number");
                    return;
                }
            }
            uint8_t id = idStr.toInt();

            // ensure name is not empty
            newName.trim();
            if (newName.length() == 0) {
                request->send(400, "text/plain", "Error: Name cannot be empty");
                return;
            }

            if (updateNodeName(id, newName.c_str())) {
                request->send(200, "text/plain", "Name updated");
            } 
            else {
                request->send(400, "text/plain", "Error: Invalid ID");
            }
        }
        else {
            request->send(400, "text/plain", "Error: Missing parameters");
        }
    });

    server->on("/web/wifi-password", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("password")) {
            String newPassword = request->getParam("password")->value();
            newPassword.trim();

            // validate password... minimum for many WiFi devices is 8 characters
            if (newPassword.length() < 8) {
                request->send(400, "text/plain", "Error: WiFi password less than 8 characters");
            } else {
                // Update WiFi Password here
                WiFi.softAP("Range-Sentinel-Gateway", newPassword);
                request->send(200, "text/plain", newPassword);
            }
        } else {
            request->send(400, "text/plain", "Error: Missing parameters");
        }
    });
}

#endif
