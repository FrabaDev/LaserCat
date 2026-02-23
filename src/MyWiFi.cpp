#include "MyWiFi.hh"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

static bool _isSTA = false;

void MyWiFi::begin() {
    String ssid, password;

    if (LittleFS.exists("/wifi_config.json")) {
        File f = LittleFS.open("/wifi_config.json", "r");
        if (f) {
            JsonDocument doc;
            deserializeJson(doc, f);
            f.close();
            ssid = doc["ssid"].as<String>();
            password = doc["password"].as<String>();
        }
    }

    if (ssid.length() > 0) {
        Serial.print("WiFi: Connecting to ");
        Serial.println(ssid);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());

        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000UL) {
            delay(250);
            Serial.print(".");
        }
        Serial.println();

        if (WiFi.status() == WL_CONNECTED) {
            _isSTA = true;
            Serial.print("WiFi: Connected, IP: ");
            Serial.println(WiFi.localIP());
            MDNS.begin("lasercat");
            Serial.println("mDNS: lasercat.local");
            return;
        }
        Serial.println("WiFi: Connection failed, starting AP");
    }

    // Start AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP("CatLaser-Setup");
    Serial.print("WiFi: AP started, IP: ");
    Serial.println(WiFi.softAPIP());
    MDNS.begin("lasercat");
    _isSTA = false;
}

bool MyWiFi::isSTA() const {
    return _isSTA;
}
