#include <arduinoOS_default.h>
ArduinoOS_default arduinoOS_default;

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

//Default Functions
void aos_help(char** param,uint8_t parCnt){
    aos.listCommands();
};
void aos_list(char** param,uint8_t parCnt){
    if(parCnt != 2){
        aos.o(aos.textInvalidParameter);
        aos.manCommand("list");
        return;
    }
    if((String)param[1] == "cmd")  aos.listCommands();
    if((String)param[1] == "var")  aos.listVariables();
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
    if(parCnt != 2){
        aos.listVariables();return;
    }
    char b[SHORT];
    if(aos.getVariable(param[1],b)){
      aos.o(b);
    }else{
        aos.o("Not Found!"); 
    }
};
void aos_set(char** param,uint8_t parCnt){
    if(parCnt != 3){
        aos.o(aos.textInvalidParameter);
        aos.manCommand("set");
        return;
    }
    if(aos.setVariable(param[1],param[2]))
      aos.o("Success!");
    else
      aos.o("Not Found!");  
};
void aos_stats(char** param,uint8_t parCnt){
    char str_temp[64];
    snprintf(str_temp,64,"       Date: %s",__DATE__ " " __TIME__);aos.o(str_temp);
    snprintf(str_temp,64,"  Free Heap: %d Bytes",freeMemory());aos.o(str_temp);
    snprintf(str_temp,64,"Used EEPROM: %d Bytes",aos._usedEeprom);aos.o(str_temp);
    //const char compile_date[] = ;
};
void aos_clear(char** param,uint8_t parCnt){
    aos.o("\033[2J\033[1;1H");
};
void aos_restart(char** param,uint8_t parCnt){
    asm volatile ("jmp 0");
};
void aos_gpio(char** param,uint8_t parCnt){
    if(parCnt < 3 || parCnt > 4 ){
        aos.o(aos.textInvalidParameter);
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
            aos.o("HIGH"); 
        else
            aos.o("LOW"); 
    }
}

void ArduinoOS_default::begin(){
    aos.addCommand("gpio",aos_gpio,"gpio [write|read] [#] [0|1]");
    aos.addCommand("help",aos_help,NULL,true);
    aos.addCommand("list",aos_list,"list [var|cmd]");
    aos.addCommand("load",aos_load,NULL,true);
    aos.addCommand("save",aos_save,"save config");
    aos.addCommand("get",aos_get,"get [parameter]");
    aos.addCommand("set",aos_set,"set [parameter] [value]");
    aos.addCommand("stats",aos_stats,NULL,true);
    aos.addCommand("clear",aos_clear,NULL,true);
    aos.addCommand("restart",aos_restart,NULL,true);
}
