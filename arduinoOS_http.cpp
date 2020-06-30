#if defined ESP8266

#include "arduinoOS_http.h"

AsyncWebServer ArduinoOS_http::server(80);

//Global
ArduinoOS_http::ArduinoOS_http():ArduinoOS_mqtt(){

};
void ArduinoOS_http::begin(){
    ArduinoOS_mqtt::begin();


    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Hello, world");
    });

    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "404");
    });

    //Files
    //webServer80.serveStatic("/favicon.ico", SPIFFS, "/www/favicon.ico"); 
    //webServer2400.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");

    server.begin();

};
void ArduinoOS_http::loop(){
    ArduinoOS_mqtt::loop();
};



#endif