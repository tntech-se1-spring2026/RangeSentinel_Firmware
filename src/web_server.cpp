#ifdef NODE_TYPE_VIEWER

#include "web_server.h"

#define HTTP_URL "http://range-sentinel.com"
#define URL "range-sentinel.com"

AsyncWebSocket ws("/ws");

AsyncMiddlewareFunction ensureURL([](AsyncWebServerRequest* request, ArMiddlewareNext next) {
    // Get the host once to save processing
    String host = request->host();

    // If they are already on the right URL, let them through
    if (host == URL || host == WiFi.softAPIP().toString()) {
        next();
    }else if(host.indexOf("firefox") > 0 || host.indexOf("msft") > 0){ // If it's a known "noisy" probe, just give them a 404 instead of redirecting (this stops the "redirect loop" crash)
        request->send(404);
    }else{ // Otherwise, redirect to your landing page
        request->redirect(HTTP_URL);
    }
});

void startWebServer(AsyncWebServer *server) {
    // Register WebSocket handler
    server->addHandler(&ws);

    // Keep the event listener for debugging and monitoring
    ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.printf("WS: Client %u connected\n", client->id());
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.printf("WS: Client %u disconnected\n", client->id());
        }
    });

    // Stop the Bootstrap .map "Death Spiral"
    // This prevents the server from hunting through LittleFS for missing files
    server->on("/css/bootstrap.min.css.map", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not Found");
    });

    // Add an interceptor for the JS map file if you use bootstrap.bundle.min.js
    server->on("/js/bootstrap.bundle.min.js.map", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not Found");
    });

    server->addMiddleware(&ensureURL);

    // Start Backend API Routes
    startBackend(server);

    // Intercept Microsoft/Windows connectivity probes
    server->on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Microsoft NCSI");
    });
    // Intercept Android/Chrome connectivity probes
    server->on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(204);
    });
    // Intercept Apple/iOS connectivity probes
    server->on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", "<html><body>Success</body></html>");
    });

    // Start File Server
    // This contains serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");
    startFileServer(server);

    // start the server
    server->begin();
    Serial.println("HTTP Server started");
}

void startFileServer(AsyncWebServer *server) {
    // Manually handle the root (/) so the user gets the homepage
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(LittleFS, "/www/index.html", "text/html");
    });

    // Serve other files normally, but WITHOUT setDefaultFile
    // Now, if a file like "canonical.html" is missing, it just returns 404
    // immediately without searching the disk 4 times.
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
                if(!WiFi.softAP("Range-Sentinel-Gateway", newPassword)) {
                    request->send(500, "text/plain", "Could not update WiFi password");
                } else {
                    request->send(200, "text/plain", newPassword);
                }
            }
        } else {
            request->send(400, "text/plain", "Error: Missing parameters");
        }
    });
}

#endif
