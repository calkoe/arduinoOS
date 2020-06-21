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
    addCommand("status",          interface_status,  "🖥 Shows System / Wifi / MQTT status",false);
    addCommand("mqtt-connect",    interface_connect, "📡 apply mqtt settings and connect to configured server",false);
    addCommand("mqtt-publish",    interface_publish, "📡 [topic] [message] | publish a message to topic",false);
   
};

void ArduinoOS_mqtt::begin(){
    ArduinoOS_wifi::begin();
    config(0);
};
void ArduinoOS_mqtt::loop(){
    ArduinoOS_wifi::loop();
    //Timer 1S
    static unsigned long t{0};
    if((unsigned long)(millis()-t)>=1000&&(t=millis())){
        if(mqtt.connected()){
            ArduinoOS::status = 5;
        }else if(enable && ArduinoOS_wifi::connected()){ 
            if(ArduinoOS::status == 5) ArduinoOS::status = 2;
            mqtt.connect(clientID.c_str(),user.c_str(),password.c_str());
        };
    };
};

//Methods
bool ArduinoOS_mqtt::config(uint8_t s){
    if(net)         delete net;
    if(netSecure)   delete netSecure;
    if(enable){
        mqtt.setKeepAlive(60);
        if(tls){
            netSecure = new WiFiClientSecure;
            if(!tlsVerify) netSecure->setInsecure();
            mqtt.begin(server.c_str(),port,*netSecure);
        }else{
            net = new WiFiClientSecure;
            mqtt.begin(server.c_str(),port,*net);
        }
    }
    return true;
}

//Interface
void ArduinoOS_mqtt::interface_status(char** c,uint8_t n){
    ArduinoOS_wifi::interface_status(c,n);
    o("");o("📡 MQTT:");

    char* m;
    switch(mqtt.lastError()){
        case LWMQTT_SUCCESS:                    m = "LWMQTT_SUCCESS"; break;
        case LWMQTT_BUFFER_TOO_SHORT:           m = "LWMQTT_BUFFER_TOO_SHORT"; break;
        case LWMQTT_VARNUM_OVERFLOW:            m = "LWMQTT_VARNUM_OVERFLOW"; break;
        case LWMQTT_NETWORK_FAILED_CONNECT:     m = "LWMQTT_NETWORK_FAILED_CONNECT"; break;
        case LWMQTT_NETWORK_TIMEOUT:            m = "LWMQTT_NETWORK_TIMEOUT"; break;
        case LWMQTT_NETWORK_FAILED_READ:        m = "LWMQTT_NETWORK_FAILED_READ"; break;
        case LWMQTT_NETWORK_FAILED_WRITE:       m = "LWMQTT_NETWORK_FAILED_WRITE"; break;
        case LWMQTT_REMAINING_LENGTH_OVERFLOW:  m = "LWMQTT_REMAINING_LENGTH_OVERFLOW"; break;
        case LWMQTT_REMAINING_LENGTH_MISMATCH:  m = "LWMQTT_REMAINING_LENGTH_MISMATCH"; break;
        case LWMQTT_MISSING_OR_WRONG_PACKET:    m = "LWMQTT_MISSING_OR_WRONG_PACKET"; break;
        case LWMQTT_CONNECTION_DENIED:          m = "LWMQTT_CONNECTION_DENIED"; break;
        case LWMQTT_FAILED_SUBSCRIPTION:        m = "LWMQTT_FAILED_SUBSCRIPTION"; break;
        case LWMQTT_SUBACK_ARRAY_OVERFLOW:      m = "LWMQTT_SUBACK_ARRAY_OVERFLOW"; break;
        case LWMQTT_PONG_TIMEOUT:               m = "LWMQTT_PONG_TIMEOUT"; break;
        default: char* m = "UNKOWN";
    };

    char* r;
    switch(mqtt.returnCode()){
        case LWMQTT_CONNECTION_ACCEPTED:        r = "LWMQTT_CONNECTION_ACCEPTED"; break;
        case LWMQTT_UNACCEPTABLE_PROTOCOL:      r = "LWMQTT_UNACCEPTABLE_PROTOCOL"; break;
        case LWMQTT_IDENTIFIER_REJECTED:        r = "LWMQTT_IDENTIFIER_REJECTED"; break;
        case LWMQTT_SERVER_UNAVAILABLE:         r = "LWMQTT_SERVER_UNAVAILABLE"; break;
        case LWMQTT_BAD_USERNAME_OR_PASSWORD:   r = "LWMQTT_BAD_USERNAME_OR_PASSWORD"; break;
        case LWMQTT_NOT_AUTHORIZED:             r = "LWMQTT_NOT_AUTHORIZED"; break;
        case LWMQTT_UNKNOWN_RETURN_CODE:        r = "LWMQTT_UNKNOWN_RETURN_CODE"; break;
        default: char* r = "UNKOWN";
    };
    snprintf(charIOBuffer,LONG,"%-20s : %s","Connected",mqtt.connected()?"true":"false");o(charIOBuffer);
    snprintf(charIOBuffer,LONG,"%-20s : %s","Last Error",m);o(charIOBuffer);
    snprintf(charIOBuffer,LONG,"%-20s : %s","Return Code",r);o(charIOBuffer);
    if(netSecure){
        char s[100];
        netSecure->getLastSSLError(s,100);
        snprintf(charIOBuffer,LONG,"%-20s : %s","Last SSL Error",s);o(charIOBuffer);
    }
};

void ArduinoOS_mqtt::interface_connect(char** c,uint8_t n){
    o("✅ Type 'status' to check status");
};

void ArduinoOS_mqtt::interface_publish(char** c,uint8_t n){
    
    mqtt.publish(c[1],c[2]);
};
#endif