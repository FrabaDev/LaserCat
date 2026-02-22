#pragma once
#ifndef _WEBINTERFACE_HH_
#define _WEBINTERFACE_HH_

#include <ESPAsyncWebServer.h>
#include "MyServo.hh"
#include "MyLaser.hh"

extern AsyncWebServer server;
extern AsyncWebSocket ws;

namespace WebInterface {
    void begin(MyServo &sA, MyServo &sB, Laser &lz,
               int &progNum, bool &loopAct, bool &firstClient);
}

#endif
