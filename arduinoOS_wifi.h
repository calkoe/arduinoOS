#if defined ESP8266

#include "arduinoOS.h"
#include <ESP8266WiFi.h> 

#pragma once

#define MAX_TELNET_CLIENTS 2

class ArduinoOS_wifi : public ArduinoOS{

    private:

        //Methods
        static void config(uint8_t);
        static bool connected();
        static int  calcRSSI(int);

        //Interface
        static void interface_status(char**,uint8_t);
        static void interface_scan(char**,uint8_t);
        static void interface_apply(char**,uint8_t);
        static void interface_ping(char**,uint8_t);

        //Telnet
        static void telnetLoop();
        static void telnetOut(void*);
        static bool telnetBlock;
        static WiFiServer TelnetServer;
        static WiFiClient TelnetClient[];

    public:

        //Global
        ArduinoOS_wifi(HardwareSerial&, unsigned int = SERSPEED);
        static void begin();
        static void loop();

        //Settings
        static bool   enableTelnet;
        static bool   sta_enable;
        static String sta_network;
        static String sta_password;
        static String sta_ip;
        static String sta_subnet;
        static String sta_gateway;
        static String sta_dns;
        static bool   ap_enable;
        static String ap_network;
        static String ap_password;

};

#endif