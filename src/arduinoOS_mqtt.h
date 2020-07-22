#if not defined ARDUINOOS_MQTT_H and defined ESP8266
#define ARDUINOOS_MQTT_H

#include "arduinoOS_wifi.h"
#include <MQTT.h>

class ArduinoOS_mqtt : public ArduinoOS_wifi{

    private:

        //Global
        static WiFiClient*          net;
        static WiFiClientSecure*    netSecure;
        static bool connected;
        struct SUB {
            char*       topic;
            int         qos;
            void        (*function)(char*,char*);
            SUB*        sub;
        };
        static SUB* sub;

        //Methods
        static bool config(uint8_t);
        static void handle(MQTTClient*, char*, char*, int);
        
    protected:

        //Interface
        static void interface_status(char**,uint8_t);
        static void interface_connect(char**,uint8_t);
        static void interface_publish(char**,uint8_t);

    public:

        //Global
        ArduinoOS_mqtt();
        static void begin();
        static void loop();

        //API MQTT
        static MQTTClient*  mqtt;
        static void publish(char*, char*, bool = false, int = 0);
        static void publish(String&, String&, bool = false, int = 0);
        static void subscripe(char*,uint8_t,void (*function)(char*,char*));
        static void unsubscripe(char*);

        //API Settings
        static bool         enable;
        static String       server;
        static int          port;
        static bool         tls;
        static bool         tlsVerify;
        static String       clientID;
        static String       user;
        static String       password;

};

#endif