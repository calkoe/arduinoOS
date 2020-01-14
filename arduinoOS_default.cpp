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
    aos.listVariables();
};
void aos_list(char** param,uint8_t parCnt){
    if(parCnt != 2){
        aos.o(aos.textInvalidParameter);
        aos.manCommand("list");
        return;
    }
    if((String)param[1] == "cmd" || (String)param[1] == "commands")  aos.listCommands();
    if((String)param[1] == "var" || (String)param[1] == "variables") aos.listVariables();
};
void aos_load(char** param,uint8_t parCnt){

};
void aos_save(char** param,uint8_t parCnt){

};
void aos_get(char** param,uint8_t parCnt){

};
void aos_set(char** param,uint8_t parCnt){

};
void aos_stats(char** param,uint8_t parCnt){
    char str_temp[64];
    snprintf(str_temp,64,"Aviable Heap: %d Bytes",freeMemory());
    aos.o(str_temp);
};
void aos_clear(char** param,uint8_t parCnt){
    aos.o("\033[2J\033[1;1H");
};
void aos_restart(char** param,uint8_t parCnt){

};

void ArduinoOS_default::begin(){
    
    aos.addCommand("help",aos_help,"help");
    aos.addCommand("list",aos_list,"list");
    aos.addCommand("load",aos_load,"load");
    aos.addCommand("save",aos_save,"save");
    aos.addCommand("get",aos_get,"get");
    aos.addCommand("set",aos_set,"set");
    aos.addCommand("stats",aos_stats,"stats");
    aos.addCommand("clear",aos_clear,"clear");
    aos.addCommand("restart",aos_restart,"restart");

}