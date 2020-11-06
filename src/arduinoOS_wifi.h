#pragma once
#ifdef ESP8266

#include "arduinoOS.h"
#include <ESP8266WiFi.h> 
#include <ESP8266httpUpdate.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <DNSServer.h>

#define MAX_TELNET_CLIENTS 2

class ArduinoOS_wifi : public ArduinoOS{

    private:

        //Methods
        static bool config(u8);
        static s32  calcRSSI(s32);

        //NTP
        static WiFiUDP   wifiUDP;
        static NTPClient timeClient;

        //Telnet
        static void telnetLoop();
        static void telnetOut(void*);
        static WiFiServer* TelnetServer;
        static WiFiClient* TelnetClient;
        
    protected:

    public:

        //Global
        ArduinoOS_wifi();
        static void begin();
        static void loop();

        //API Settings
        static bool   telnet_enable;
        static bool   ntp_enable;
        static String ntp_server;
        static s32    ntp_offset;
        static bool   sta_enable;
        static String sta_network;
        static String sta_password;
        static String sta_ip;
        static String sta_subnet;
        static String sta_gateway;
        static String sta_dns;
        static bool   sta_connected();
        static bool   ap_enable;
        static String ap_network;
        static String ap_password;

};

#endif