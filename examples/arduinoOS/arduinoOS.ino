#include <arduinoOS.h>
ArduinoOS aos

bool    demoBool{true};
int     demoInt{1234};
double  demoDouble{1234.1234};
String  demoString{"DEMO"};

void setup()
{
    aos.variableAdd("demo/bool",    demoBool);
    aos.variableAdd("demo/int",     demoInt);
    aos.variableAdd("demo/double",  demoDouble);
    aos.variableAdd("demo/string",  demoString, "..comment..", false, false);

    aos.commandAdd("demo",[](char** param,u8 parCnt){
        Serial.println("Hello from Demo Function");
        Serial.println("Parameter count: " + (String)parCnt);
        for(int i{0};i<parCnt;i++){
            Serial.println("- " + (String)param[i]);
        }
    });

    aos.setInterval([](){
        aos.o("Every Second");
    },1000);


    aos.setTimeout([](){
        aos.o("Once after 5 Seconds");
    },5000);

    aos.begin();
}

void loop()
{ 
    aos.loop();
}