#include <arduinoOS_wifi.h>
ArduinoOS_wifi aos{Serial};

bool    demoBool{true};
int     demoInt{1234};
double  demoDouble{1234.1234};
String  demoString{"DEMO"};

void demo(char** param,uint8_t parCnt){
    aos.o("Hello from Demo Function",true);
    aos.o("Parameter count: " + (String)parCnt),true;
    for(int i{0};i<parCnt;i++){
        aos.o("- " + (String)param[i],true);
    }
}

void setup()
{
    aos.addVariable("demo/bool",    demoBool);
    aos.addVariable("demo/int",     demoInt);
    aos.addVariable("demo/double",  demoDouble);
    aos.addVariable("demo/string",  demoString, "..comment..", false, false);
    aos.addCommand("demo",demo,"demo [parm1] [parm2] - ..comment..",false);
    aos.begin();
}

void loop()
{ 
    aos.loop();
}