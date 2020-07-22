#include <arduinoOS_mqtt.h>
#include <ArduinoJson.h>



ArduinoOS_mqtt aos;
StaticJsonDocument<200> doc;

void handle(char* topic,char* payload){
    aos.o(topic);
    aos.o(payload);
};

void setup()
{
    aos.begin();
    aos.subscripe("demo",2,handle);
    aos.unsubscripe("demo");
}

void loop()
{ 
    aos.loop();
}