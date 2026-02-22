#include <Arduino.h>
#include <LittleFS.h>
#include <ElegantOTA.h>
#include "MyWiFi.hh"
#include "MyLaser.hh"
#include "MyServo.hh"
#include "MyProgramController.hh"
#include "WebInterface.hh"

// GPIO pins (NodeMCU ESP8266)
static const int servoPinA = 14;  // D5
static const int servoPinB = 12;  // D6
static const int laserPin  = 15;  // D8 — vía transistor NPN

MyServo servoA;
MyServo servoB;
Laser   laser;
MyWiFi  mywifi;

int  programNumber      = 0;
bool loopActive         = false;
bool firstClientConnected = false;

void setupServos() {
    servoA.setup(servoPinA, 0.0, "Servo A", 0,   90,  544, 1550);
    servoB.setup(servoPinB, 0.0, "Servo B", -85, 85);
}

void setupLaser() {
    laser.setup(laserPin);
    laser.onWithDelay();
    laser.offWithDelay();
}

void setup() {
    Serial.begin(115200);
    delay(100);

    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed!");
    } else {
        Serial.println("LittleFS mounted");
    }

    mywifi.begin();
    WebInterface::begin(servoA, servoB, laser,
                        programNumber, loopActive, firstClientConnected);

    setupServos();
    setupLaser();

    delay(1000);
    servoA.initMove();
    servoB.initMove();
}

void loop() {
    static MyProgramController progCont(servoA, servoB, laser);

    ElegantOTA.loop();
    ws.cleanupClients();

    if (!firstClientConnected && millis() > 120000UL) {
        progCont.randProgram();
    }

    if (loopActive) {
        progCont.program(programNumber);
    }

    yield();
}
