#pragma once
#ifdef ESP8266

#include <arduinoOS_wifi.h>
#include <MQTT.h>

class ArduinoOS_mqtt : public ArduinoOS_wifi{

    private:

        //Global
        static WiFiClient*          net;
        static WiFiClientSecure*    netSecure;
        static bool connected;
        struct SUB {
            char*       topic;
            u8          qos;
            void        (*function)(char*,char*);
            SUB*        sub;
        };
        static SUB* sub;

        //Methods
        static bool config(u8);
        static void handle(MQTTClient*, char*, char*, s32);
        
    protected:

        //Interface
        static void interface_status(char**,u8);
        static void interface_connect(char**,u8);
        static void interface_publish(char**,u8);

    public:

        //Global
        ArduinoOS_mqtt();
        static void begin();
        static void loop();

        //API MQTT
        static MQTTClient*  mqtt;
        static void publish(char*, char*, bool = false, u8 = 0);
        static void publish(String&, String&, bool = false, u8 = 0);
        static void subscripe(char*,u8,void (*function)(char*,char*));
        static void unsubscripe(char*);

        //API Settings
        static bool         enable;
        static String       server;
        static s32          port;
        static bool         tls;
        static bool         tlsVerify;
        static String       clientID;
        static String       user;
        static String       password;

};

#endif