#if defined ESP8266

#include "arduinoOS_wifi.h"
#include "../mqtt/MQTTClient.h"

#pragma once

class ArduinoOS_mqtt : public ArduinoOS_wifi{

    private:
        static WiFiClient*          net;
        static WiFiClientSecure*    netSecure;


        //Methods
        static bool config(uint8_t);



    public:

        //Global
        ArduinoOS_mqtt();
        static void begin();
        static void loop();


        //Settings
        static MQTTClient   mqtt;
        static bool         enable;
        static String       server;
        static int          port;
        static bool         tls;
        static bool         tlsVerify;
        static String       clientID;
        static String       user;
        static String       password;
    
        //Interface
        static void interface_status(char**,uint8_t);
        static void interface_connect(char**,uint8_t);
        static void interface_publish(char**,uint8_t);

};

#endif