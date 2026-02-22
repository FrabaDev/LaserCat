#include "WebInterface.hh"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ElegantOTA.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

static MyServo  *_servoA       = nullptr;
static MyServo  *_servoB       = nullptr;
static Laser    *_laser        = nullptr;
static int      *_programNumber = nullptr;
static bool     *_loopActive   = nullptr;
static bool     *_firstClient  = nullptr;

static void broadcastState() {
    JsonDocument doc;
    doc["servoA"]   = _servoA->getAngle();
    doc["servoB"]   = _servoB->getAngle();
    doc["laser"]    = _laser->getState();
    doc["program"]  = *_programNumber;
    doc["loop"]     = *_loopActive;
    String msg;
    serializeJson(doc, msg);
    ws.textAll(msg);
}

static void onWsMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (!info->final || info->index != 0 || info->len != len) return;
    if (info->opcode != WS_TEXT) return;

    JsonDocument doc;
    if (deserializeJson(doc, data, len) != DeserializationError::Ok) return;

    const char *cmd = doc["cmd"];
    if (!cmd) return;

    if (strcmp(cmd, "servo") == 0) {
        const char *axis = doc["axis"];
        float angle = doc["angle"].as<float>();
        if (axis && strcmp(axis, "A") == 0) _servoA->moveAbs(angle);
        else if (axis && strcmp(axis, "B") == 0) _servoB->moveAbs(angle);
    } else if (strcmp(cmd, "laser") == 0) {
        _laser->setState(doc["state"].as<bool>());
    } else if (strcmp(cmd, "program") == 0) {
        *_programNumber = doc["number"].as<int>();
    } else if (strcmp(cmd, "loop") == 0) {
        *_loopActive = doc["active"].as<bool>();
    }

    *_firstClient = true;
    broadcastState();
}

static void onWsEvent(AsyncWebSocket *srv, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        broadcastState();
    } else if (type == WS_EVT_DATA) {
        onWsMessage(arg, data, len);
    }
}

namespace WebInterface {

void begin(MyServo &sA, MyServo &sB, Laser &lz,
           int &progNum, bool &loopAct, bool &firstClient) {
    _servoA        = &sA;
    _servoB        = &sB;
    _laser         = &lz;
    _programNumber = &progNum;
    _loopActive    = &loopAct;
    _firstClient   = &firstClient;

    // WebSocket
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    // Serve index.html
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
        req->send(LittleFS, "/index.html", "text/html");
    });

    // Status JSON
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *req) {
        JsonDocument doc;
        doc["servoA"]  = _servoA->getAngle();
        doc["servoB"]  = _servoB->getAngle();
        doc["laser"]   = _laser->getState();
        doc["program"] = *_programNumber;
        doc["loop"]    = *_loopActive;
        String out;
        serializeJson(doc, out);
        req->send(200, "application/json", out);
    });

    // Save WiFi credentials and restart
    server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *req) {
        String ssid, pass;
        if (req->hasParam("ssid", true))
            ssid = req->getParam("ssid", true)->value();
        if (req->hasParam("password", true))
            pass = req->getParam("password", true)->value();

        JsonDocument doc;
        doc["ssid"]     = ssid;
        doc["password"] = pass;
        File f = LittleFS.open("/wifi_config.json", "w");
        if (f) {
            serializeJson(doc, f);
            f.close();
        }
        req->send(200, "text/plain", "Credenciales guardadas. Reiniciando...");
        delay(500);
        ESP.restart();
    });

    // OTA panel
    ElegantOTA.begin(&server);

    server.begin();
    Serial.println("HTTP server started");
}

} // namespace WebInterface
