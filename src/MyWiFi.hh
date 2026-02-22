#pragma once
#ifndef _MYWIFI_HH_
#define _MYWIFI_HH_

#include <Arduino.h>

class MyWiFi {
public:
    void begin();
    bool isSTA() const;
};

#endif
