#include <arduino.h>
#include "arduinoOS.h"
#include <ESP8266WiFi.h> 
#include <Math.h>

#pragma once

class arduinoOS_wifi{

    private:


    public:

        //Global
        arduinoOS_wifi();
        void begin();
        void loop();

        //System
        uint8_t s{5};

        //Static Settings
        uint8_t statusLed{0};

        //Dynamic Settings
        bool   sta_enabled{true};
        String sta_network{};
        String sta_password{};
        String sta_ip{};
        String sta_subnet{};
        String sta_gateway{};
        String sta_dns{};
        
        bool   ap_enabled{false};
        String ap_network{"arduinoOS"};
        String ap_password{"12345678"};

        //Interface
        static void status(char**,uint8_t);
        static void sta_scan(char**,uint8_t);
        static void sta_connect(char**,uint8_t);
        static void sta_ping(char**,uint8_t);

        //Methods


};
extern arduinoOS_wifi aos_wifi;