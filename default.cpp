#include <arduinoOS.h>

#if not defined ESP8266
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
int ArduinoOS::freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
#endif
#if defined ESP8266
  int ArduinoOS::freeMemory() { return ESP.getFreeHeap(); };
#endif

//Default Functions
void ArduinoOS::aos_gpio(char** param,uint8_t parCnt){
    if(parCnt < 3 || parCnt > 4 ){
        aos.p(textInvalidParameter);
        aos.manCommand("gpio");
        return;
    }
    if(strstr(param[1],"w")){
        pinMode(atoi(param[2]),OUTPUT);
        digitalWrite(atoi(param[2]),atoi(param[3]));
    }
    if(strstr(param[1],"r")){
        pinMode(atoi(param[2]),INPUT);
        if(digitalRead(atoi(param[2])))
            aos.o("1"); 
        else
            aos.o("0"); 
    }
}
void ArduinoOS::aos_help(char** param,uint8_t parCnt){
    if(parCnt == 1)
      aos.listCommands();
    if(parCnt == 2)
      aos.listCommands(param[1]);
};
void ArduinoOS::aos_load(char** param,uint8_t parCnt){
    aos.loadVariables(false);
    aos.p(textOk);
};
void ArduinoOS::aos_save(char** param,uint8_t parCnt){
    aos.loadVariables(true);
    aos.p(textOk);
};
void ArduinoOS::aos_get(char** param,uint8_t parCnt){
    char b[SHORT];
    if(parCnt == 2){
      if(aos.getVariable(param[1],b))
        aos.o(b);
      else
        aos.listVariables(param[1]);
    }else
      aos.listVariables();
};
void ArduinoOS::aos_set(char** param,uint8_t parCnt){
    if(parCnt != 3){
        aos.p(textInvalidParameter);
        aos.manCommand("set");
        return;
    }
    if(aos.setVariable(param[1],param[2])){
      aos.loadVariables(true);
      aos.p(textOk);
    }else{
      aos.p(textNotFound); 
    } 
};
void ArduinoOS::aos_stats(char** param,uint8_t parCnt){
    char str_temp[32];
    snprintf(str_temp,32,"  Date: %s",aos.aos_date.c_str());aos.o(str_temp);
    snprintf(str_temp,32,"  Heap: %d B",aos.freeMemory());aos.o(str_temp);
    snprintf(str_temp,32,"EEPROM: %d B",aos.usedEeprom);aos.o(str_temp);
};
void ArduinoOS::aos_clear(char** param,uint8_t parCnt){
    aos.p(textEscClear);
};
void ArduinoOS::aos_reboot(char** param,uint8_t parCnt){
    for(int i{0};;i++){aos.o('.',false);delay(333);}
};
void ArduinoOS::aos_reset(char** param,uint8_t parCnt){
    aos.aos_date = "";
    aos.loadVariables(true);
    aos_reboot(0,0);
}

void ArduinoOS::defaultInit(){
    aos.addCommand("gpio",aos_gpio,"gpio [w|r] [pin] [0|1]");
    aos.addCommand("help",aos_help,(char)0,true);
    aos.addCommand("load",aos_load,(char)0,true);
    aos.addCommand("save",aos_save,(char)0,true);
    aos.addCommand("get",aos_get,"get [?]");
    aos.addCommand("set",aos_set,"set [par] [val]");
    aos.addCommand("stats",aos_stats,"-");
    aos.addCommand("clear",aos_clear,(char)0,true);
    aos.addCommand("reboot",aos_reboot,"-");
    aos.addCommand("reset",aos_reset,"-");
};