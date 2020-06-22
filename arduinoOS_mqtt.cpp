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
    addVariable("mqtt/enable",    enable    ,"📡 Enable MQTT");
    addVariable("mqtt/server",    server    ,"📡 MQTT Server IP or Name");
    addVariable("mqtt/port",      port      ,"📡 MQTT Server Port");
    addVariable("mqtt/tls",       tls       ,"📡 Use TLS");
    addVariable("mqtt/tlsVerify", tlsVerify ,"📡 Verify TLS Certificates");
    addVariable("mqtt/clientID",  clientID  ,"📡 Client ID");
    addVariable("mqtt/user",      user      ,"📡 Username");
    addVariable("mqtt/password",  password  ,"📡 Password");
    addCommand("status",          interface_status,  "🖥  Shows System / Wifi / MQTT status",false);
    addCommand("mqtt-connect",    interface_connect, "📡 Apply mqtt settings and connect to configured server",false);
    addCommand("mqtt-publish",    interface_publish, "📡 [topic] [message] | publish a message to topic",false);
   
};

void ArduinoOS_mqtt::begin(){
    ArduinoOS_wifi::begin();
    config(0);
};
void ArduinoOS_mqtt::loop(){
    ArduinoOS_wifi::loop();
    if(enable)  mqtt.loop();
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
        mqtt.setOptions(60, true, 1000);
        if(tls){
            netSecure = new WiFiClientSecure;
            if(!tlsVerify) netSecure->setInsecure();
            mqtt.begin(server.c_str(),port,*netSecure);
        }else{
            net = new WiFiClientSecure;
            mqtt.begin(server.c_str(),port,*net);
        };
    }else{
        mqtt.disconnect();
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
    snprintf(charIOBuffer,LONG,"Set  mqtt/enabled: %s","true");o(charIOBuffer);
    enable  = true;
    if(n>=2){
            snprintf(charIOBuffer,LONG,"Set  mqtt/server: %s",c[1]);o(charIOBuffer);
            setVariable("mqtt/server",c[1]);
    };
    if(n>=3){
            snprintf(charIOBuffer,LONG,"Set mqtt/port: %s",c[2]);o(charIOBuffer);
            setVariable("mqtt/port",c[2]);
    };
    if(n>=4){
            snprintf(charIOBuffer,LONG,"Set mqtt/user: %s",c[2]);o(charIOBuffer);
            setVariable("mqtt/user",c[3]);
    };
    if(n>=5){
            snprintf(charIOBuffer,LONG,"Set mqtt/password: %s",c[2]);o(charIOBuffer);
            setVariable("mqtt/password",c[4]);
    };
    loadVariables(true);
    o("✅ Type 'status' to check status");
    config(1);
};

void ArduinoOS_mqtt::interface_publish(char** c,uint8_t n){
    if(n>=3){
        mqtt.publish(c[1],c[2]);
        o("📨 sent!");
    };
};
#endif