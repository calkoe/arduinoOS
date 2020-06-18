#if defined ESP8266

#include "arduinoOS_mqtt.h"

WiFiClient*         ArduinoOS_mqtt::net;
WiFiClientSecure*   ArduinoOS_mqtt::netSecure;
MQTTClient          ArduinoOS_mqtt::mqtt;
bool                ArduinoOS_mqtt::enable{true};
String              ArduinoOS_mqtt::server{"192.168.1.7"};
int                 ArduinoOS_mqtt::port{1883};
bool                ArduinoOS_mqtt::tls{false};
bool                ArduinoOS_mqtt::tlsVerify{false};
String              ArduinoOS_mqtt::clientID{ArduinoOS::aos_hostname};
String              ArduinoOS_mqtt::user{};
String              ArduinoOS_mqtt::password{};
//Global
ArduinoOS_mqtt::ArduinoOS_mqtt():ArduinoOS_wifi(){
    addVariable("mqtt/enable",    enable);
    addVariable("mqtt/server",    server);
    addVariable("mqtt/port",      port);
    addVariable("mqtt/tls",       tls);
    addVariable("mqtt/tlsVerify", tlsVerify);
    addVariable("mqtt/clientID",  clientID);
    addVariable("mqtt/user",      user);
    addVariable("mqtt/password",  password);
    addCommand("mqtt",            interface_mqtt);
};

void ArduinoOS_mqtt::begin(){
    ArduinoOS_wifi::begin();
    //config(0);
};
void ArduinoOS_mqtt::loop(){
    ArduinoOS_wifi::loop();
};

//Methods
bool ArduinoOS_mqtt::config(uint8_t s){
    if(net)         delete net;
    if(netSecure)   delete netSecure;
    if(tls){
        netSecure = new WiFiClientSecure;
        if(!tlsVerify) netSecure->setInsecure();
        mqtt.begin(server.c_str(),port,*netSecure);
    }else{
        net = new WiFiClientSecure;
        mqtt.begin(server.c_str(),port,*net);
    }
    mqtt.connect(clientID.c_str());
    return true;
}

//Interface
void ArduinoOS_mqtt::interface_mqtt(char** c,uint8_t n){
    snprintf(charIOBuffer,LONG,"✅ connecting");o(charIOBuffer);
    config(0);
}

#endif