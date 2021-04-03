#pragma once
#if defined ESP8266 || defined ESP32

#include "arduinoOS_wifi.h"
#include <MQTT.h>

class ArduinoOS_mqtt : public ArduinoOS_wifi{

    private:

        //Global
        static WiFiClient*          net;
        static WiFiClientSecure*    netSecure;
        struct SUB {
            char*       topic;
            u8          qos;
            void        (*function)(char*,char*);
            SUB*        sub;
        };
        static SUB* sub;
        static bool retry;
        #if defined ESP32 
            static SemaphoreHandle_t xBinarySemaphore;
        #endif

        //Methods
        static void handle(MQTTClient*, char*, char*, s16);
        static void mqtt_daemon();
        static void mqtt_config();
                
    protected:

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
        static void disconnect();

        //API
        static bool         mqtt_connected;
        static bool         mqtt_enable;
        static String       mqtt_server;
        static s16          mqtt_port;
        static bool         mqtt_tls;
        static bool         mqtt_tlsVerify;
        static String       mqtt_CACert;
        static String       mqtt_certificate;
        static String       mqtt_privateKey;
        static String       mqtt_clientID;
        static String       mqtt_user;
        static String       mqtt_password;
        static u16          mqtt_retryIntervall;
        static bool         connected();




};

#endif