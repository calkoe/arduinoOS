#if defined ESP8266 || defined ESP32

#include "arduinoOS_mqtt.h"

bool                    ArduinoOS_mqtt::mqtt_connected{false};
ArduinoOS_mqtt::SUB*    ArduinoOS_mqtt::sub{nullptr};
WiFiClient*             ArduinoOS_mqtt::net;
WiFiClientSecure*       ArduinoOS_mqtt::netSecure;
MQTTClient*             ArduinoOS_mqtt::mqtt;
bool                    ArduinoOS_mqtt::mqtt_enable{false};
String                  ArduinoOS_mqtt::mqtt_server{};
s16                     ArduinoOS_mqtt::mqtt_port{1883};
bool                    ArduinoOS_mqtt::mqtt_tls{false};
bool                    ArduinoOS_mqtt::mqtt_tlsVerify{false};
String                  ArduinoOS_mqtt::mqtt_CACert;
String                  ArduinoOS_mqtt::mqtt_certificate;
String                  ArduinoOS_mqtt::mqtt_privateKey;
String                  ArduinoOS_mqtt::mqtt_user{};
String                  ArduinoOS_mqtt::mqtt_password{};

void ArduinoOS_mqtt::begin(){
    ArduinoOS_wifi::begin();
    connect(false);
    // DAEMON 5ms
    #if defined ESP8266
        setInterval([](){
            daemon();
        },5,"mqttDaemon");
    #endif
    #if defined ESP32 
        xTaskCreatePinnedToCore([](void* arg){
            while(true){
                ArduinoOS_mqtt::daemon();
                vTaskDelay(5);
            }
        }, "mqttDaemon", 4096, NULL, 1, NULL, 1);
    #endif
};

void ArduinoOS_mqtt::loop(){
    ArduinoOS_wifi::loop();
};

void ArduinoOS_mqtt::daemon(){
    if(mqtt && mqtt_enable){
        if(ArduinoOS::status == 2 &&  mqtt->connected()) ArduinoOS::status = 5; 
        if(ArduinoOS::status == 5 && !mqtt->connected()) ArduinoOS::status = 2; 
        if(mqtt_server){
            if(mqtt->connected() && !mqtt_connected){
                mqtt_connected = true;
                eventEmit("mqttConnected",NULL);
                SUB* t = sub;
                while(t){
                    mqtt->subscribe(t->topic,t->qos);
                    t = t->sub;
                }
            }
            if(!mqtt->connected() && mqtt_connected){
                mqtt_connected = false;
                eventEmit("mqttDisconnected",NULL);
            }
            if(!mqtt_connected && ArduinoOS_wifi::connected()){ 
                connect(true);
            };
        }
        mqtt->loop();
    }
};

//Methods
void ArduinoOS_mqtt::connect(bool blocking){
    if(mqtt)        delete mqtt;
    if(net)         delete net;
    if(netSecure)   delete netSecure;
    if(mqtt_enable && mqtt_server && mqtt_port){
        mqtt = new MQTTClient;
        mqtt->setOptions(60, true, 5000);
        mqtt->onMessageAdvanced(handle);
        if(mqtt_tls){
            netSecure = new WiFiClientSecure;
            netSecure->setTimeout(5000);
            #if defined ESP8266
                if(!mqtt_tlsVerify) netSecure->setInsecure();
            #endif
            mqtt->begin(mqtt_server.c_str(),mqtt_port,*netSecure);
        }else{
            net = new WiFiClient;
            net->setTimeout(5000);
            mqtt->begin(mqtt_server.c_str(),mqtt_port,*net);
        };
        if(blocking){
            eventEmit("mqttConnecting",NULL);
            mqtt->connect(hostname.c_str(),mqtt_user.c_str(),mqtt_password.c_str());
        }
    }
};
void ArduinoOS_mqtt::publish(char* topic, char* payload, bool retained, u8 qos){
    if(mqtt) mqtt->publish(topic,payload,retained,qos);
    yield();
};
void ArduinoOS_mqtt::publish(String& topic, String& payload, bool retained, u8 qos){
    if(mqtt) mqtt->publish(topic,payload,retained,qos);
    yield();
};
void ArduinoOS_mqtt::subscripe(char* topic,u8 qos,void (*function)(char*,char*)){
    if(mqtt && mqtt->connected()) mqtt->subscribe(topic,qos);
    SUB* b = new SUB{topic,qos,function,sub};
    sub = b;
    yield();
};
void ArduinoOS_mqtt::unsubscripe(char* topic){
    if(mqtt && mqtt->connected()) mqtt->unsubscribe(topic);
    SUB* t = sub;
    SUB* l = sub;
    while(t){
        if(!strcmp(t->topic,topic)){
            if(t == sub){
                sub = t->sub;
                delete t;
            }else{
                l->sub = t->sub;
                delete t;
                t = l->sub;
            }
        }else{
            l = t;
            t = t->sub;
        }
    }
    yield();
};
void ArduinoOS_mqtt::handle(MQTTClient *client, char* topic, char* payload, s16 payload_length){
    SUB* t = sub;
    while(t){
        if(!strcmp(t->topic,topic))
            (*(t->function))(topic,payload);
        t = t->sub;
    };
};

bool ArduinoOS_mqtt::connected(){
    return (mqtt && mqtt_enable && mqtt->connected());
}


ArduinoOS_mqtt::ArduinoOS_mqtt():ArduinoOS_wifi(){
    variableAdd("mqtt/enable",      mqtt_enable         ,           "📡 mqtt_enable MQTT");
    variableAdd("mqtt/server",      mqtt_server         ,           "📡 MQTT mqtt_server IP or Name");
    variableAdd("mqtt/port",        mqtt_port           ,           "📡 MQTT mqtt_server Port");
    variableAdd("mqtt/tls",         mqtt_tls            ,           "📡 Use TLS");
    variableAdd("mqtt/tlsVerify",   mqtt_tlsVerify      ,           "📡 Verify TLS Certificates");
    //variableAdd("mqtt/CACert",      mqtt_CACert         ,           "📡 TLS CACert");
    //variableAdd("mqtt/certificate", mqtt_certificate    ,           "📡 TLS Certificate");
    //variableAdd("mqtt/privateKey",  mqtt_privateKey     ,           "📡 TLS privateKey");
    variableAdd("mqtt/user",        mqtt_user           ,           "📡 Username");
    variableAdd("mqtt/password",    mqtt_password       ,           "📡 Password");
    commandAdd("mqttStatus",[](char** param,u8 parCnt){
        if(!mqtt || !mqtt_enable){
            o("");o("📡 MQTT Disabled!");
            return;
        }
        o("");o("📡 MQTT:");
        const char* m{""};
        switch(mqtt->lastError()){
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
            default: m = "UNKOWN";
        };
        const char* r{""};
        switch(mqtt->returnCode()){
            case LWMQTT_CONNECTION_ACCEPTED:        r = "LWMQTT_CONNECTION_ACCEPTED"; break;
            case LWMQTT_UNACCEPTABLE_PROTOCOL:      r = "LWMQTT_UNACCEPTABLE_PROTOCOL"; break;
            case LWMQTT_IDENTIFIER_REJECTED:        r = "LWMQTT_IDENTIFIER_REJECTED"; break;
            case LWMQTT_SERVER_UNAVAILABLE:         r = "LWMQTT_SERVER_UNAVAILABLE"; break;
            case LWMQTT_BAD_USERNAME_OR_PASSWORD:   r = "LWMQTT_BAD_USERNAME_OR_PASSWORD"; break;
            case LWMQTT_NOT_AUTHORIZED:             r = "LWMQTT_NOT_AUTHORIZED"; break;
            case LWMQTT_UNKNOWN_RETURN_CODE:        r = "LWMQTT_UNKNOWN_RETURN_CODE"; break;
            default: r = "UNKOWN";
        };
        snprintf(OUT,LONG,"%-20s : %s","mqtt_connected",mqtt->connected()?"true":"false");o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","Last Error",m);o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","Return Code",r);o(OUT);
        if(netSecure){
            char s[100];
            #if defined ESP8266 
                netSecure->getLastSSLError(s,100); 
            #endif
            #if defined ESP32
                netSecure->lastError(s,100); 
            #endif
            snprintf(OUT,LONG,"%-20s : %s","Last SSL Error",s);o(OUT);
        }
    },"📡 Shows System / Wifi / MQTT status",false);

    commandAdd("mqttConnect",[](char** param,u8 parCnt){
        if(parCnt>=2){
                snprintf(OUT,LONG,"Set  mqtt/enabled: %s","true");o(OUT);
                mqtt_enable  = true;
                snprintf(OUT,LONG,"Set  mqtt/server: %s",param[1]);o(OUT);
                variableSet("mqtt/server",param[1]);
        };
        if(parCnt>=3){
                snprintf(OUT,LONG,"Set mqtt/port: %s",param[2]);o(OUT);
                variableSet("mqtt/port",param[2]);
        };
        if(parCnt>=4){
                snprintf(OUT,LONG,"Set mqtt/user: %s",param[3]);o(OUT);
                variableSet("mqtt/user",param[3]);
        };
        if(parCnt>=5){
                snprintf(OUT,LONG,"Set mqtt/password: %s",param[4]);o(OUT);
                variableSet("mqtt/password",param[4]);
        };
        if(parCnt==6){
                snprintf(OUT,LONG,"Set mqtt/tls: %s",param[5]);o(OUT);
                variableSet("mqtt/tls",param[5]);
        };
        variableLoad(true);
        o("DONE! ✅ > Type 'mqttStatus' to check status");
        connect(false);
    }, "📡 [mqtt_server] [mqtt_port] [mqtt_user] [mqtt_password] [tls] | Apply mqtt settings and connect to configured mqtt_server",false);

    commandAdd("mqttPublish",[](char** param,u8 parCnt){
        if(parCnt>=3){
            mqtt->publish(param[1],param[2]);
            o("📨 sent!");
        };
    },    "📡 [topic] [message] | publish a message to topic",false);

};

#endif