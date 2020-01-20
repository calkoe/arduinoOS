#include <arduinoOS_default.h>

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
//int freeMemory() { return 0; };


//Default Functions
void aos_gpio(char** param,uint8_t parCnt){
    if(parCnt < 3 || parCnt > 4 ){
        aos.p(textInvalidParameter);
        aos.manCommand("gpio");
        return;
    }
    if((String)param[1] == "write"){
        pinMode(atoi(param[2]),OUTPUT);
        digitalWrite(atoi(param[2]),atoi(param[3]));
    }
    if((String)param[1] == "read"){
        pinMode(atoi(param[2]),INPUT);
        if(digitalRead(atoi(param[2])))
            aos.o("1"); 
        else
            aos.o("0"); 
    }
}
void aos_help(char** param,uint8_t parCnt){
    aos.listCommands(param[1]);
};
void aos_load(char** param,uint8_t parCnt){
    aos.loadVariables(false);
    aos.o("ok");
};
void aos_save(char** param,uint8_t parCnt){
    aos.loadVariables(true);
    aos.o("ok");
};
void aos_get(char** param,uint8_t parCnt){
    char b[SHORT];
    if(aos.getVariable(param[1],b)){
      aos.o(b);
    }else{
      aos.listVariables(param[1]);
    }
};
void aos_set(char** param,uint8_t parCnt){
    if(parCnt != 3){
        aos.p(textInvalidParameter);
        aos.manCommand("set");
        return;
    }
    if(aos.setVariable(param[1],param[2])){
      aos.loadVariables(true);
      aos.o("ok");
    }else{
      aos.p(textNotFound); 
    } 
};
void aos_stats(char** param,uint8_t parCnt){
    char str_temp[64];
    snprintf(str_temp,64,"  Date: %s",aos_date.c_str());aos.o(str_temp);
    snprintf(str_temp,64,"  Heap: %d B",freeMemory());aos.o(str_temp);
    snprintf(str_temp,64,"EEPROM: %d B",aos.usedEeprom);aos.o(str_temp);
};
void aos_clear(char** param,uint8_t parCnt){
    aos.p(textEscClear);
};
void aos_reboot(char** param,uint8_t parCnt){
    for(int i{0};;i++){aos.o('.',false);delay(333);}
};
void aos_reset(char** param,uint8_t parCnt){
    aos_date = "";
    aos.loadVariables(true);
    aos_reboot(NULL,NULL);
}

ArduinoOS_default::ArduinoOS_default(){
    aos.addCommand("gpio",aos_gpio,"gpio [write|read] [#] [0|1]");
    aos.addCommand("help",aos_help,NULL,true);
    aos.addCommand("load",aos_load,NULL,true);
    aos.addCommand("save",aos_save,NULL,true);
    aos.addCommand("get",aos_get,"get [fiter]");
    aos.addCommand("set",aos_set,"set [parameter] [value]");
    aos.addCommand("stats",aos_stats,"-");
    aos.addCommand("clear",aos_clear,NULL,true);
    aos.addCommand("reboot",aos_reboot,"-");
    aos.addCommand("reset",aos_reset,"-");
}
