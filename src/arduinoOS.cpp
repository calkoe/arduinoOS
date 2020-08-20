#include <arduinoOS.h>

//Global
bool                ArduinoOS::isBegin = false;
HardwareSerial      ArduinoOS::serialInstance{Serial};
ArduinoOS::AOS_CMD* ArduinoOS::aos_cmd{nullptr};
ArduinoOS::AOS_VAR* ArduinoOS::aos_var{nullptr};
ArduinoOS::AOS_EVT* ArduinoOS::aos_evt{nullptr};
uint8_t             ArduinoOS::IOC{0};
unsigned            ArduinoOS::IOP[]{0};
char                ArduinoOS::IO[][LONG]{0};
char                ArduinoOS::OUT[LONG]{0};
uint8_t             ArduinoOS::status{5};
unsigned            ArduinoOS::usedEeprom{0};
bool                ArduinoOS::serialEcho{true};
bool                ArduinoOS::enableWatchdog{true};
bool                ArduinoOS::enableSerial{true};
unsigned int        ArduinoOS::serialBaud{SERSPEED};
bool                ArduinoOS::autoLoad{true};
bool                ArduinoOS::autoReset{true};
bool                ArduinoOS::locked{false};
uint8_t             ArduinoOS::statusLed{STATUSLED};
String              ArduinoOS::date{__DATE__ " " __TIME__};
String              ArduinoOS::date_temp{date};
String              ArduinoOS::hostname{"arduinoOS"};
String              ArduinoOS::password{"root"};
String              ArduinoOS::firmware{"generic"};
ArduinoOS::ArduinoOS(){
    addVariable("sys/date", date,"",true,true);
    addVariable("sys/hostname", hostname,"");
    addVariable("sys/password", password,"");
    addCommand("gpio",aos_gpio,"🖥  gpio [pin] [0|1]");
    addCommand("help",aos_help,"",true);
    addCommand("load",aos_load,"",true);
    addCommand("save",aos_save,"",true);
    addCommand("get",aos_get,"🖥  get [filter]");
    addCommand("set",aos_set,"🖥  set [par] [val]");
    addCommand("status",aos_stats,"🖥");
    addCommand("clear",aos_clear,"",true);
    addCommand("lock",aos_lock,"🖥");
    addCommand("reboot",aos_reboot,"🖥");
    addCommand("reset",aos_reset,"🖥");
}
void ArduinoOS::begin(){
    isBegin         = true;
    if(enableSerial) serialInstance.begin(serialBaud);
    if(enableWatchdog) wdt_enable(WDTO_4S);
    #if defined ESP8266 || defined ESP32 
        EEPROM.begin(EEPROM_SIZE);
    #endif
    if(statusLed) pinMode(statusLed,OUTPUT);
    if(autoLoad)  loadVariables();
    o(0x07,false);
    p(textEscClear);
    p(textWelcome);
    o(firmware);
    listCommands();
    terminalNl(true);
};
void ArduinoOS::loop(){
    //Read Serial
    while(serialInstance.available()){
        while(serialInstance.available())
            charIn(serialInstance.read(),true);
        delay(1);
    }
    //Watchdog
    wdt_reset();
    //Events
    loopEvent();
    //Status LED
    if(statusLed){
        static unsigned long t{0};
        if(!status)          digitalWrite(statusLed,1);
        else if(status==5)   digitalWrite(statusLed,0);
        else if((unsigned long)(millis()-t)>=200&&(t=millis())){
            static bool o{1};
            static uint8_t p{0};
            if(p<2*status) digitalWrite(statusLed,o=!o);
            if(p==10){p=0;o=1;}else p++;
        }
    };
};
//Events
void ArduinoOS::listenEvent(char* name,void (*function)(void*)){
    AOS_EVT* e = new AOS_EVT{name,function,false,nullptr,nullptr};
    if(aos_evt == nullptr){
        aos_evt = e;
    }else{
        AOS_EVT* i{aos_evt};
        while(i->aos_evt != nullptr) i = i->aos_evt;
        i->aos_evt = e;
    }
};
void ArduinoOS::emitEvent(char* name,void* value,bool now){
    AOS_EVT* i{aos_evt};
    while(i != nullptr){
        if(!strcmp(i->name,name)){
            if(now){
                (*(i->function))(value);
                i->active  = false;
            }else{
                i->value   = value;
                i->active  = true;
            }
        }
        i = i->aos_evt;
    };
};
void ArduinoOS::loopEvent(){
    AOS_EVT* i{aos_evt};
    while(i != nullptr){
        if(i->active){
            (*(i->function))(i->value);
            i->active = false;
        }
        i = i->aos_evt;
    };
};

//Commands
bool ArduinoOS::addCommand(char* name,void (*function)(char** param, uint8_t parCnt),char* description,bool hidden){
    AOS_CMD* b = new AOS_CMD{name,function,description,hidden,nullptr};
    if(aos_cmd == nullptr){
        aos_cmd = b;
    }else{
        AOS_CMD* i{aos_cmd};
        while(i->aos_cmd != nullptr){
            if(!strcmp(i->name,name)){
                i->function     = function;
                i->description  = description;
                i->hidden       = hidden;
                delete b;
                return true;
            };    
            i = i->aos_cmd;
        };
        i->aos_cmd = b;
    }
    return false;
};
void ArduinoOS::listCommands(char* filter){
    p(textCommands);
    AOS_CMD* i{aos_cmd};
    while(i != nullptr){
        if(!i->hidden && (filter=="" || strstr(i->name, filter))){
            snprintf(OUT,LONG, "%-20s %s",i->name,i->description);o(OUT);
        }
        i = i->aos_cmd;
    };
}
void ArduinoOS::manCommand(char* name){
    AOS_CMD* i{aos_cmd};
    while(i != nullptr){
        if(!strcmp(i->name,name)){
            o(i->description);
            return;
        }
        i = i->aos_cmd;
    };
    return;
}
bool ArduinoOS::callCommand(char* name,char** param, uint8_t parCnt){
    AOS_CMD* i{aos_cmd};
    while(i != nullptr){
        if(!strncmp(i->name,name,SHORT)){
            (*(i->function))(param, parCnt);
            return true;
        }
        i = i->aos_cmd;
    };
    return false;
}

//Variables
bool ArduinoOS::addVariable(char* n,bool& v,char* d,bool h,bool p)  {return _addVariable(n,&v,d,h,p,AOS_DT_BOOL);};
bool ArduinoOS::addVariable(char* n,int& v,char* d,bool h,bool p)   {return _addVariable(n,&v,d,h,p,AOS_DT_INT);};
bool ArduinoOS::addVariable(char* n,double& v,char* d,bool h,bool p){return _addVariable(n,&v,d,h,p,AOS_DT_DOUBLE);};
bool ArduinoOS::addVariable(char* n,String& v,char* d,bool h,bool p){return _addVariable(n,&v,d,h,p,AOS_DT_STRING);};
bool ArduinoOS::_addVariable(char* name,void* value,char* description,bool hidden,bool protect,AOS_DT aos_dt){
    if(isBegin){
        p(textErrorBegin);
        return false;
    }
    AOS_VAR* b = new AOS_VAR{name,value,description,hidden,protect,aos_dt,nullptr};
    if(aos_var == nullptr){
        aos_var = b;
    }else{
        AOS_VAR* i{aos_var};
        while(i->aos_var != nullptr){
            if(!strcmp(i->name,name)){
                i->value        = value;
                i->description  = description;
                i->hidden       = hidden;
                i->protect      = protect;
                i->aos_dt       = aos_dt;
                delete b;
                return true;
            }
            i = i->aos_var;
        };
        i->aos_var = b;
    } 
    return false;
};
void ArduinoOS::listVariables(char* filter){
    p(textVariables);
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(!i->hidden && (filter=="" || strstr(i->name, filter))){ 
            if(i->aos_dt==AOS_DT_BOOL)   snprintf(OUT,LONG,"%-20s : %-20s\t%s %s", i->name,*(bool*)(i->value) ? "true" : "false",i->description,(i->protect ? "(Protected)":""));
            if(i->aos_dt==AOS_DT_INT)    snprintf(OUT,LONG,"%-20s : %-20d\t%s %s", i->name,*(int*)(i->value),i->description,(i->protect ? "(Protected)":""));
            if(i->aos_dt==AOS_DT_DOUBLE) {char str_temp[SHORT];dtostrf(*(double*)(i->value), 4, 2, str_temp);snprintf(OUT,LONG,"%-20s : %-20s\t%s %s", i->name,str_temp,i->description,(i->protect ? "(Protected)":""));};
            if(i->aos_dt==AOS_DT_STRING) snprintf(OUT,LONG,"%-20s : %-20s\t%s %s", i->name,(*(String*)(i->value)).c_str(),i->description,(i->protect ? "(Protected)":""));
            o(OUT);
        }
        i = i->aos_var;
    };
}
bool ArduinoOS::setVariable(char* name,char* value){
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(!strcmp(i->name,name)){
            if(i->protect) return false;
            if(i->aos_dt==AOS_DT_BOOL)      *(bool*)(i->value)   = !value ? false   : (!strcmp(value,"1") || !strcmp(value,"true")) ? true : false;
            if(i->aos_dt==AOS_DT_INT)       *(int*)(i->value)    = !value ? 0       : atoi(value);
            if(i->aos_dt==AOS_DT_DOUBLE)    *(double*)(i->value) = !value ? 0.0     : atof(value); 
            if(i->aos_dt==AOS_DT_STRING)    *(String*)(i->value) = !value ? 0       : value;
            return true;
        }
        i = i->aos_var;
    };
    return false;
};
void* ArduinoOS::getValue(char* name){
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(!strcmp(i->name,name)) return i->value;
        i = i->aos_var;
    };
    return nullptr;
};
void ArduinoOS::loadVariables(bool save){
    AOS_VAR* i{aos_var};unsigned int p{0};
    while(i != nullptr){
            if(i->aos_dt==AOS_DT_BOOL){
                if(save)EEPROM.put(p,*(bool*)(i->value));
                else    EEPROM.get(p,*(bool*)(i->value));
                p+=sizeof(bool);
            } 
            if(i->aos_dt==AOS_DT_INT){
                if(save)EEPROM.put(p,*(int*)(i->value));
                else    EEPROM.get(p,*(int*)(i->value));
                p+=sizeof(int);
            } 
            if(i->aos_dt==AOS_DT_DOUBLE){
                if(save)EEPROM.put(p,*(double*)(i->value));
                else    EEPROM.get(p,*(double*)(i->value));
                p+=sizeof(double);
            }
            if(i->aos_dt==AOS_DT_STRING){
                if(save){
                    for(unsigned int s{0};s<(*(String*)(i->value)).length();s++,p++){
                        EEPROM.write(p,(*(String*)(i->value))[s]);
                    }
                    EEPROM.write(p,(char)0);p++;
                }else{
                    *(String*)(i->value) = "";
                    while(true){
                        char b = EEPROM.read(p);p++;
                        if((uint8_t)b == (char)0) break;
                        *(String*)(i->value)+=b; 
                        if((*(String*)(i->value)).length() >= LONG) break;
                    }
                }
            }
            if((date != date_temp && autoReset || date == "!") && !save){
                o("RESET");date=date_temp;
                loadVariables(true);return;
            }
        i = i->aos_var;
    };
    usedEeprom = p;
    #if defined ESP8266 || defined ESP32 
        if(save) EEPROM.commit();
    #endif
};

//Interface
void ArduinoOS::o(const char* ca,bool nl){
    if(ca){
        if(enableSerial) serialInstance.print(ca);
        emitEvent("o",(void*)ca,true);
    }
    if(nl) o("\r\n",false);
};
void ArduinoOS::o(const char c,bool nl){
    char caBuffer[2] = {c,0};o(caBuffer,nl);
};
void ArduinoOS::o(String s,bool nl){
    o(s.c_str(),nl);
};
void ArduinoOS::p(const char* ca,bool nl){
    char caBuffer[2] = {0,0};
    for (unsigned int k = 0; k < strlen_P(ca); k++){
        caBuffer[0] = pgm_read_byte_near(ca + k);
        o(caBuffer,false);
    }o("",nl);
};
void ArduinoOS::charIn(char c,bool echo){
        //o((String)(int)c,false);
        static char lc{c};
        if(charEsc(c)) return;                                      //Filter ESC 
        if(c != 13 && c != 10 && echo) o(c,false);                  //No Echo CR + NL
        if(c == 127 || c == 8){                                     //DEL BACKSPACE
            if(IOP[IOC] > 0){
                IO[IOC][--IOP[IOC]] = 0;
                terminalLine();
            }else{
                o(7,false);                                         //BELL
            }
        }else if(c == 9){}                                          //TAB
        else if(c != 13 && c != 10 && IOP[IOC] < LONG-1){           //CR NL
            IO[IOC][IOP[IOC]++] = c;                    
        }else if(!(lc == 13 && c == 10)){                           //CR NL 
            o(13,false);o(10,false);                                //CR NL
            if(IOP[IOC]>0 && !locked) terminalParseCommand();
            if(!strcmp(IO[IOC],password.c_str())) locked = false;
            IOC = !IOC;
            terminalNl(true);
        }
        lc=c;
};
bool ArduinoOS::charEsc(char c){
    static AOS_ESC charInEsc{ESC_STATE_NONE};
    bool ret = false;
    if(c == 27 || c == 255){
        charInEsc = ESC_STATE_START;
        ret = true;
    }else if(charInEsc == ESC_STATE_START){
        if(c == 91 || c == 251 || c == 253){
            charInEsc = ESC_STATE_CODE;
            ret = true;
        }else
            charInEsc = ESC_STATE_NONE;
    }else if(charInEsc == ESC_STATE_CODE){
        switch(c){
            case 65:  //Cursor Up
                IOC = !IOC;
                terminalLine();
                break;
            case 66:  //Cursor Down
                IOC = !IOC;
                terminalLine();
                break;
            /*case 0x43:  //Cursor Right
                if(IOP[IOC] < LONG-1){
                    o("\e[1C",false);
                } IOP[IOC]++;
                break;
            case 0x44:  //Cursor Left
                if(IOP[IOC] > 0){
                    IOP[IOC]--;
                    o("\e[1D",false);
                }

                break;
            case 0x30:  //Cursor OK
                break;
            case 0x6e:  //Cursor Closing
                break;
            */
        };
        charInEsc = ESC_STATE_NONE;
        ret = true;
    }
    return ret;
}
void ArduinoOS::clearBuffer(char* ca,unsigned int l){
    for(unsigned int i{0};i<l;i++)ca[i]=0;IOP[IOC]=0;
};
void ArduinoOS::terminalNl(bool clear){
    if(locked){
        p(textEnterPassword,false);
    }else{
        snprintf(OUT,LONG,"%s:/>",hostname.c_str());o(OUT,false);
    };
    if(clear) clearBuffer(IO[IOC],LONG);
}
void ArduinoOS::terminalLine(){
    o("\e[2K",false);o(0x0D,false);
    terminalNl(false);
    o(IO[IOC],false);
}
void ArduinoOS::terminalParseCommand(){
    uint8_t parCnt{0};
    char*   param[SHORT]{NULL};
    unsigned s{0};
    for(unsigned i{0};i<LONG;i++){
        s = i;
        while(IO[IOC][i] != 32 && IO[IOC][i]){
            if(IO[IOC][i] == 34 && IO[IOC][i-1] != 27){
                s = ++i;
                while((IO[IOC][i] != 34 || IO[IOC][i-1] == 27) && IO[IOC][i]) i++;
                break;
            }
            i++;
        };
        if(i!=s){
            char* buffer = (char*)malloc(i-s+1);
            for(unsigned j{0};j<(i-s);j++) buffer[j] = IO[IOC][s+j];
            buffer[i-s] = 0;param[parCnt++] = buffer;
        }
        if(!IO[IOC][i]) break;
    }

    if(parCnt>0) if(!callCommand(param[0],param,parCnt)) p(textCommandNotFound);
    for(uint8_t i{0};i<parCnt;i++) delete param[i];
};

//API

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
    if(parCnt == 2){
      pinMode(atoi(param[1]),INPUT);
        if(digitalRead(atoi(param[1])))
            o("1"); 
        else
            o("0"); 
    }else if(parCnt == 3){
      pinMode(atoi(param[1]),OUTPUT);
      digitalWrite(atoi(param[1]),atoi(param[2]));
    }else{
      p(textInvalidParameter);
      manCommand("gpio");
      return;
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
    o("🖥  System:");
    snprintf(OUT,LONG,"%-20s : %s","FIRMWARE",firmware.c_str());o(OUT);
    snprintf(OUT,LONG,"%-20s : %s","COMPILED",date.c_str());o(OUT);
    snprintf(OUT,LONG,"%-20s : %d B","HEAP",freeMemory());o(OUT);
    snprintf(OUT,LONG,"%-20s : %d B","EERPOM",usedEeprom);o(OUT);
};
void ArduinoOS::aos_clear(char** param,uint8_t parCnt){
    p(textEscClear);
};
void ArduinoOS::aos_lock(char** param,uint8_t parCnt){
    locked = true;
};
void ArduinoOS::aos_reboot(char** param,uint8_t parCnt){
    #if defined ESP8266
      ESP.restart();
    #endif
    for(int i{0};;i++){o('.',false);delay(333);}
};
void ArduinoOS::aos_reset(char** param,uint8_t parCnt){
    date = "!";
    loadVariables(true);
    aos_reboot(0,0);
}