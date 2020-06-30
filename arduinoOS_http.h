#pragma once
#if defined ESP8266

#include "arduinoOS_mqtt.h"
#include <ESPAsyncTCP.h>
#include "ESPAsyncWebServer.h"

class ArduinoOS_http : public ArduinoOS_mqtt{

    private:

        //Global
        static AsyncWebServer server;

    

    protected:




    public:

        //Global
        ArduinoOS_http();
        static void begin();
        static void loop();






};

#endif