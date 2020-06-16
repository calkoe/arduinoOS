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

//Interface Methods
void ArduinoOS::aos_gpio(char** param,uint8_t parCnt){
    if(parCnt < 3 || parCnt > 4 ){
        p(textInvalidParameter);
        manCommand("gpio");
        return;
    }
    if(strstr(param[1],"w")){
        pinMode(atoi(param[2]),OUTPUT);
        digitalWrite(atoi(param[2]),atoi(param[3]));
    }
    if(strstr(param[1],"r")){
        pinMode(atoi(param[2]),INPUT);
        if(digitalRead(atoi(param[2])))
            o("1"); 
        else
            o("0"); 
    }
}
void ArduinoOS::aos_help(char** param,uint8_t parCnt){
    if(parCnt == 1)
      listCommands();
    if(parCnt == 2)
      listCommands(param[1]);
};
void ArduinoOS::aos_load(char** param,uint8_t parCnt){
    loadVariables(false);
    p(textOk);
};
void ArduinoOS::aos_save(char** param,uint8_t parCnt){
    loadVariables(true);
    p(textOk);
};
void ArduinoOS::aos_get(char** param,uint8_t parCnt){
    if(parCnt == 2)
        listVariables(param[1]);
    else
        listVariables();
};
void ArduinoOS::aos_set(char** param,uint8_t parCnt){
    if(parCnt < 2 || parCnt > 3){
        p(textInvalidParameter);
        manCommand("set");
        return;
    }
    if(setVariable(param[1],param[2])){
      loadVariables(true);
      p(textOk);
    }else{
      p(textNotFound); 
    } 
};
void ArduinoOS::aos_stats(char** param,uint8_t parCnt){
    snprintf(charIOBuffer,LONG,"%-20s : %s","COMPILED",aos_date.c_str());o(charIOBuffer);
    snprintf(charIOBuffer,LONG,"%-20s : %d B","HEAP",freeMemory());o(charIOBuffer);
    snprintf(charIOBuffer,LONG,"%-20s : %d B","EERPOM",usedEeprom);o(charIOBuffer);
};
void ArduinoOS::aos_clear(char** param,uint8_t parCnt){
    p(textEscClear);
};
void ArduinoOS::aos_reboot(char** param,uint8_t parCnt){
    #if defined ESP8266
      ESP.restart();
    #endif
    for(int i{0};;i++){o('.',false);delay(333);}
};
void ArduinoOS::aos_reset(char** param,uint8_t parCnt){
    aos_date = "";
    loadVariables(true);
    aos_reboot(0,0);
}