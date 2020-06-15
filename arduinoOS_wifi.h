#include "arduinoOS.h"
#include <ESP8266WiFi.h> 

#pragma once

class ArduinoOS_wifi : public ArduinoOS{

    private:

        //Methods
        static void wifi_status(char**,uint8_t);
        static void wifi_config(uint8_t);
        static int  calcRSSI(int);

        //Commands
        static void sta_scan(char**,uint8_t);
        static void sta_connect(char**,uint8_t);
        static void sta_ping(char**,uint8_t);

    public:

        //Global
        ArduinoOS_wifi(HardwareSerial&, unsigned int = SERSPEED);
        static void begin();
        static void loop();

        //Settings
        static bool   sta_enabled;
        static String sta_network;
        static String sta_password;
        static String sta_ip;
        static String sta_subnet;
        static String sta_gateway;
        static String sta_dns;
        static bool   ap_enabled;
        static String ap_network;
        static String ap_password;

};