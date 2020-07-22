#ifndef ARDUINOOS_WIFI_H && defined ESP8266
#define ARDUINOOS_WIFI_H

#include "arduinoOS.h"
#include <ESP8266WiFi.h> 
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <DNSServer.h>

#define MAX_TELNET_CLIENTS 2

class ArduinoOS_wifi : public ArduinoOS{

    private:

        //Methods
        static bool config(uint8_t);
        static int  calcRSSI(int);

        //NTP
        static WiFiUDP   wifiUDP;
        static NTPClient timeClient;

        //DNS
        static DNSServer dnsServer;

        //Telnet
        static void telnetLoop();
        static void telnetOut(void*);
        static WiFiServer* TelnetServer;
        static WiFiClient* TelnetClient;
        
    protected:

        //Interface
        static void interface_status(char**,uint8_t);
        static void interface_scan(char**,uint8_t);
        static void interface_connect(char**,uint8_t);
        static void interface_ping(char**,uint8_t);

    public:

        //Global
        ArduinoOS_wifi();
        static void begin();
        static void loop();
        static bool connected();

        //API Settings
        static bool   telnet_enable;
        static String ntp_server;
        static int    ntp_offset;
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