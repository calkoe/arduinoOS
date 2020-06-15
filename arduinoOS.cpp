#include <arduinoOS.h>

//Global
bool                ArduinoOS::isBegin = false;
HardwareSerial*     ArduinoOS::serialInstance{nullptr};
ArduinoOS::AOS_CMD* ArduinoOS::aos_cmd{nullptr};
ArduinoOS::AOS_VAR* ArduinoOS::aos_var{nullptr};
ArduinoOS::AOS_EVT* ArduinoOS::aos_evt{nullptr};
unsigned            ArduinoOS::charIOBufferPos{0};
char                ArduinoOS::terminalHistory[LONG];
char                ArduinoOS::charIOBuffer[LONG];
uint8_t             ArduinoOS::status{0};
uint8_t             ArduinoOS::statusLed{0};
unsigned            ArduinoOS::usedEeprom{0};
bool                ArduinoOS::serialEcho{true};
bool                ArduinoOS::enableWatchdog{true};
bool                ArduinoOS::enableSerial{true};
bool                ArduinoOS::autoLoad{true};
bool                ArduinoOS::autoReset{true};
bool                ArduinoOS::locked{false};
String              ArduinoOS::aos_date{__DATE__ " " __TIME__};
String              ArduinoOS::aos_date_temp{aos_date};
String              ArduinoOS::aos_hostname{"arduinoOS"};
String              ArduinoOS::aos_user{"root"};
String              ArduinoOS::aos_password{"root"};
ArduinoOS::ArduinoOS(HardwareSerial& Serial,unsigned int baud){
    serialInstance  = &Serial;
    if(enableSerial) serialInstance->begin(baud);
    addVariable("sys/date", aos_date,"",true,true);
    addVariable("sys/hostname", aos_hostname,"");
    addVariable("sys/user", aos_user,"");
    addVariable("sys/password", aos_password,"");
    addCommand("gpio",aos_gpio,"🖥  gpio [w|r] [pin] [0|1]");
    addCommand("help",aos_help,"",true);
    addCommand("load",aos_load,"",true);
    addCommand("save",aos_save,"",true);
    addCommand("get",aos_get,"🖥  get [filter]");
    addCommand("set",aos_set,"🖥  set [par] [val]");
    addCommand("status",aos_stats,"🖥");
    addCommand("clear",aos_clear,"",true);
    addCommand("reboot",aos_reboot,"",true);
    addCommand("reset",aos_reset,"",true);
}
void ArduinoOS::begin(){
    isBegin         = true;
    if(enableWatchdog) wdt_enable(WDTO_4S);
    #if defined ESP8266 || defined ESP32 
        EEPROM.begin(EEPROM_SIZE);
    #endif
    if(autoLoad) loadVariables();
    o(0x07,false);
    p(textEscClear);
    p(textWelcome);
    listCommands();
    terminalNl();
};
void ArduinoOS::loop(){
    //Read Serial
    while(Serial.available()){
        while(Serial.available()){
            char c{Serial.read()};
            if(!charEsc(c)){
                o(c,false); 
                charIn(c);
            }
        }
        delay(5);
    }
    //Watchdog
    wdt_reset();
    //Events
    loopEvent();
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
        if(i->name == name){
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
            if(!strcmp(i->name,name)) return false;
            i = i->aos_cmd;
        };
        i->aos_cmd = b;
    }
    return true;
};
void ArduinoOS::listCommands(char* filter){
    p(textCommands);
    AOS_CMD* i{aos_cmd};
    while(i != nullptr){
        if(!i->hidden && (filter=="" || strstr(i->name, filter))){
            snprintf(charIOBuffer,LONG, "%-20s : %s",i->name,i->description);o(charIOBuffer);
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
            if(!strcmp(i->name,name)) return false;
            i = i->aos_var;
        };
        i->aos_var = b;
    } 
    return true;
};
void ArduinoOS::listVariables(char* filter){
    p(textVariables);
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(!i->hidden && (filter=="" || strstr(i->name, filter))){ 
            if(i->aos_dt==AOS_DT_BOOL)   snprintf(charIOBuffer,LONG,"%-20s : %-20s\t%s %s", i->name,*(bool*)(i->value) ? "true" : "false",i->description,(i->protect ? "(Protected)":""));
            if(i->aos_dt==AOS_DT_INT)    snprintf(charIOBuffer,LONG,"%-20s : %-20d\t%s %s", i->name,*(int*)(i->value),i->description,(i->protect ? "(Protected)":""));
            if(i->aos_dt==AOS_DT_DOUBLE) {char str_temp[SHORT];dtostrf(*(double*)(i->value), 4, 2, str_temp);snprintf(charIOBuffer,LONG,"%-20s : %-20s\t%s %s", i->name,str_temp,i->description,(i->protect ? "(Protected)":""));};
            if(i->aos_dt==AOS_DT_STRING) snprintf(charIOBuffer,LONG,"%-20s : %-20s\t%s %s", i->name,(*(String*)(i->value)).c_str(),i->description,(i->protect ? "(Protected)":""));
            o(charIOBuffer);
        }
        i = i->aos_var;
    };
}
bool ArduinoOS::setVariable(char* name,char* value){
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(!strcmp(i->name,name)){
            if(i->protect) return false;
            if(i->aos_dt==AOS_DT_BOOL)      *(bool*)(i->value)   = (!strcmp(value,"1") || !strcmp(value,"true")) ? true : false;
            if(i->aos_dt==AOS_DT_INT)       *(int*)(i->value)    = atoi(value);
            if(i->aos_dt==AOS_DT_DOUBLE)    *(double*)(i->value) = atof(value); 
            if(i->aos_dt==AOS_DT_STRING)    *(String*)(i->value) = value;
            return true;
        }
        i = i->aos_var;
    };
    return false;
};
void* ArduinoOS::getValue(char* name){
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(!strcmp(i->name,name))
            return (bool*)(i->value);
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
            if(aos_date != aos_date_temp && !save && autoReset){
                o("RESET");aos_date=aos_date_temp;
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
    if(enableSerial){
        serialInstance->print(ca);   
        if(nl) serialInstance->print("\r\n");
    }
    emitEvent("o",(void*)ca,true);
    if(nl) emitEvent("o",(char*)"\r\n",true);
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
void ArduinoOS::charIn(char c){
        if(c == 0x7F || c == 0x08){                                     //DEL BACKSPACE
            if(charIOBufferPos > 0){
                o(0x08,false);                                          //BACKSPACE
                o(0x20,false);                                          //SPACE
                o(0x08,false);                                          //BACKSPACE
                charIOBuffer[--charIOBufferPos] = 0;
            }else{
                o(0x07,false);                                          //BELL
            }
        }else if(c == 0x09){}                                           //TAB
        else if(c != 0x0A && c != 0x0D && charIOBufferPos < LONG-1){    //NL+CR
            charIOBuffer[charIOBufferPos++] = c;
        }else if(c == 0x0D){
            o(0x0A,false);
            if(!strcmp(charIOBuffer,"logout")) locked = true;
            if(charIOBufferPos>0 && !locked){
                strcpy(terminalHistory,charIOBuffer);
                terminalParseCommand(); 
            } 
            if(!strcmp(charIOBuffer,aos_password.c_str())) locked = false;
            charIOBufferPos=0;
            terminalNl();
        }
};
bool ArduinoOS::charEsc(char c){
    static AOS_ESC charInEsc{ESC_STATE_NONE};
    bool ret = false;
    if(c == 27){
        charInEsc = ESC_STATE_START;
        ret = true;
    }else if(charInEsc == ESC_STATE_START){
        if(c == 0x5B){
            charInEsc = ESC_STATE_CODE;
            ret = true;
        }else
            charInEsc = ESC_STATE_NONE;
    }else if(charInEsc == ESC_STATE_CODE){
        switch(c){
            case 0x41:  //Cursor Up
                terminalHandleHistory(true);
                break;
            case 0x42:  //Cursor Down
                terminalHandleHistory(false);
                break;
            /*
            case 0x43:  //Cursor Right
                break;
            case 0x44:  //Cursor Left
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
    for(unsigned int i{0};i<l;i++)ca[i]=0;
};
void ArduinoOS::terminalNl(){
    if(locked){
        delay(1000);
        p(textEnterPassword,false);
    }else{
        snprintf(charIOBuffer,LONG,"%s:/>",aos_user.c_str());o(charIOBuffer,false);
    };
    clearBuffer(charIOBuffer,LONG);
}
void ArduinoOS::terminalHandleHistory(bool u){
    o("\33[2K",false);o(0x0D,false);
    if(u){
        snprintf(charIOBuffer,LONG,"%s:/>%s",aos_user.c_str(),terminalHistory);o(charIOBuffer,false);
        strcpy(charIOBuffer,terminalHistory);
        charIOBufferPos=strlen(terminalHistory);
    }else{
        snprintf(charIOBuffer,LONG,"%s:/>",aos_user.c_str());o(charIOBuffer,false);
        charIOBufferPos=0;
        clearBuffer(charIOBuffer,LONG);
    }
}
void ArduinoOS::terminalParseCommand(){
    uint8_t parCnt{0};
    char*   param[SHORT]{NULL};
    unsigned s{0};
    for(unsigned i{0};i<LONG;i++){
        s = i;
        while(charIOBuffer[i] != 32 && charIOBuffer[i]){
            if(charIOBuffer[i] == 34 && charIOBuffer[i-1] != 27){
                s = ++i;
                while((charIOBuffer[i] != 34 || charIOBuffer[i-1] == 27) && charIOBuffer[i]) i++;
                break;
            }
            i++;
        };
        if(i!=s){
            char* buffer = (char*)malloc(i-s+1);
            for(unsigned j{0};j<(i-s);j++) buffer[j] = charIOBuffer[s+j];
            buffer[i-s] = 0;param[parCnt++] = buffer;
        }
        if(!charIOBuffer[i]) break;
    }

    if(parCnt>0) if(!callCommand(param[0],param,parCnt)) p(textCommandNotFound);
    for(uint8_t i{0};i<parCnt;i++) delete param[i];
};