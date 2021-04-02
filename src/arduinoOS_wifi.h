#pragma once
#if defined ESP8266 || defined ESP32

#if defined ESP8266
    #include <ESP8266WiFi.h> 
    #include <ESP8266httpUpdate.h>
    #define MAX_TELNET_CLIENTS 1
#endif

#if defined ESP32
    #include <WiFi.h> 
    #include <WiFiClient.h> 
    #include <WiFiClientSecure.h> 
    #include <HTTPUpdate.h>
    #include <Update.h>
    #define MAX_TELNET_CLIENTS 4
#endif

#include "arduinoOS.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <DNSServer.h>

class ArduinoOS_wifi : public ArduinoOS{

    private:

        //Methods
        static bool config(u8);
        static s16  calcRSSI(s32);

        //NTP
        static WiFiUDP*     timeClientUDP;
        static NTPClient*   timeClient;

        //Telnet
        static void telnetOut(void*);
        static WiFiServer* TelnetServer;
        static WiFiClient* TelnetClient;

        static void daemon();

    protected:

    public:

        //Global
        ArduinoOS_wifi();
        static void begin();
        static void loop();

        //API
        static bool   telnet_enable;
        static bool   ntp_enable;
        static String ntp_server;
        static s16    ntp_offset;
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
        static bool   connected();

};

#endif