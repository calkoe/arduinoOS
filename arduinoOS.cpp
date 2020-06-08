#include <arduinoOS.h>
ArduinoOS aos;

//Global
ArduinoOS::ArduinoOS(){}
void ArduinoOS::begin(HardwareSerial& Serial,unsigned int baud){
    addVariable("sys/date", aos_date,(char)0,true,false);
    addVariable("sys/name", aos_name,(char)0,false,false);
    addVariable("sys/password", aos_password,(char)0,true,false);
    isBegin         = true;
    serialInstance  = &Serial;
    if(enableSerial) serialInstance->begin(baud);
    if(enableWatchdog) wdt_enable(WDTO_4S);
    #if defined ESP8266 || defined ESP32 
        EEPROM.begin(EEPROM_SIZE);
    #endif
    if(autoLoad) loadVariables();
    defaultInit();
    o(0x07,false,true);
    p(textEscClear);
    p(textWelcome);
    listCommands();
    terminalNl();
};
void ArduinoOS::loop(){
    //Read Serial
    while(Serial.available()){
        while(Serial.available())  
            charIn(Serial.read());
        delay(5);
    }
    //Watchdog
    wdt_reset();
    //Events
    loopEvent();
};

//Events
void ArduinoOS::listenEvent(char* name,void (*function)(char)){
    AOS_EVT* e = new AOS_EVT{name,function,false,0,nullptr};
    if(aos_evt == nullptr){
        aos_evt = e;
    }else{
        AOS_EVT* i{aos_evt};
        while(i->aos_evt != nullptr) i = i->aos_evt;
        i->aos_evt = e;
    }
};
void ArduinoOS::emitEvent(char* name,char payload,bool now){
    AOS_EVT* i{aos_evt};
    while(i != nullptr){
        if(i->name == name){
            if(now){
                (*(i->function))(payload);
                i->active  = false;
            }else{
                i->payload = payload;
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
            (*(i->function))(i->payload);
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
            if(strstr(i->name,name)) return false;
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
        if(!i->hidden && (filter==(char)0 || strstr(i->name, filter))){
            snprintf(charIOBuffer,LONG, "%15s : %s",i->name,i->description);o(charIOBuffer);
        }
        i = i->aos_cmd;
    };
}
void ArduinoOS::manCommand(char* name){
    AOS_CMD* i{aos_cmd};
    while(i != nullptr){
        if(strstr(i->name,name)){
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
        if(strstr(i->name,name)){
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
            if(strstr(i->name,name)) return false;
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
        if(!i->hidden && (filter==(char)0 || strstr(i->name, filter))){ 
            if(i->aos_dt==AOS_DT_BOOL)   snprintf(charIOBuffer,LONG,"%15s : %12s\t\t%s %s", i->name,*(bool*)(i->value) ? "true" : "false",i->description,(i->protect ? "(Protected)":""));
            if(i->aos_dt==AOS_DT_INT)    snprintf(charIOBuffer,LONG,"%15s : %12d\t\t%s %s", i->name,*(int*)(i->value),i->description,(i->protect ? "(Protected)":""));
            if(i->aos_dt==AOS_DT_DOUBLE) {char str_temp[SHORT];dtostrf(*(double*)(i->value), 4, 2, str_temp);snprintf(charIOBuffer,LONG,"%15s : %12s\t\t%s %s", i->name,str_temp,i->description,(i->protect ? "(Protected)":""));};
            if(i->aos_dt==AOS_DT_STRING) snprintf(charIOBuffer,LONG,"%15s : %12s\t\t%s %s", i->name,(*(String*)(i->value)).c_str(),i->description,(i->protect ? "(Protected)":""));
            o(charIOBuffer);
        }
        i = i->aos_var;
    };
}
bool ArduinoOS::setVariable(char* name,char* value){
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(strstr(i->name,name)){
            if(i->protect) return false;
            if(i->aos_dt==AOS_DT_BOOL)      *(bool*)(i->value)   = (strstr(i->name,"1") || strstr(i->name,"true")) ? true : false;
            if(i->aos_dt==AOS_DT_INT)       *(int*)(i->value)    = atoi(value);
            if(i->aos_dt==AOS_DT_DOUBLE)    *(double*)(i->value) = atof(value); 
            if(i->aos_dt==AOS_DT_STRING)    *(String*)(i->value) = value;
            return true;
        }
        i = i->aos_var;
    };
    return false;
};
bool ArduinoOS::getVariable(char* name, char* b){
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(strstr(i->name,name)){
            if(i->aos_dt==AOS_DT_BOOL)   sprintf(b,"%s",*(bool*)(i->value) ? "true" : "false");
            if(i->aos_dt==AOS_DT_INT)    sprintf(b,"%d",*(int*)(i->value));
            if(i->aos_dt==AOS_DT_DOUBLE){char str_temp[SHORT];dtostrf(*(double*)(i->value), 4, 2, str_temp);sprintf(b,"%s",str_temp);};
            if(i->aos_dt==AOS_DT_STRING) sprintf(b,"%s",(*(String*)(i->value)).c_str());
            return true;
        }
        i = i->aos_var;
    };
    return false;
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
                    EEPROM.write(p,0);p++;
                }else{
                    *(String*)(i->value) = "";
                    while(true){
                        char b = EEPROM.read(p);p++;
                        if((uint8_t)b == 0) break;
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
void ArduinoOS::o(const char c,bool nl, bool esc){
    char caBuffer[2] = {c,0};o(caBuffer,nl,esc);
};
void ArduinoOS::o(const char* ca,bool nl, bool esc){
    if(nl)  serialInstance->println(ca);   else    serialInstance->print(ca);
};
void ArduinoOS::o(String s,bool nl, bool esc){
    if(nl)  serialInstance->println(s);   else    serialInstance->print(s);
};
void ArduinoOS::p(const char* ca,bool nl, bool esc){
    char caBuffer[2] = {0,0};
    for (unsigned int k = 0; k < strlen_P(ca); k++){
        caBuffer[0] = pgm_read_byte_near(ca + k);
        o(caBuffer,false,esc);
    }o("",nl,esc);
};
void ArduinoOS::charIn(char c){
        static char charIOBufferLast;
        if(charEsc(c)) return;
        if(c == 0x7F || c == 0x08){                                     //DEL BACKSPACE
            if(charIOBufferPos > 0){
                o(0x08,false,true);                                     //BACKSPACE
                o(0x20,false,true);                                     //SPACE
                o(0x08,false,true);                                     //BACKSPACE
                charIOBuffer[--charIOBufferPos] = 0;
            }else{
                o(0x07,false,true);                                     //BELL
            }
        }else if(c == 0x09){                                            //TAB
        }
        else if(c != 0x0A && c != 0x0D && charIOBufferPos < LONG-1){    //NL+CR
            if(serialEcho) o(c,false);
            charIOBuffer[charIOBufferPos++] = c;
        }else{
            if((c == 0x0A && charIOBufferLast == 0x0D) || (c == 0x0D && charIOBufferLast == 0x0A)) return;
            o("",true,true);
            if(strstr(charIOBuffer,"logout")) locked = true;
            if(charIOBufferPos>0 && !locked){
                strcpy(terminalHistory,charIOBuffer);
                terminalParseCommand(); 
            } 
            if(strstr(charIOBuffer,aos_password.c_str())) locked = false;
            charIOBufferPos=0;
            terminalNl();
        }
    charIOBufferLast = c;
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
        if(c == 0x41){          // A    Cursor Up
            terminalHandleHistory(true);
            charInEsc = ESC_STATE_NONE;
        }else if(c == 0x42){    // B    Cursor Down
            terminalHandleHistory(false);
            charInEsc = ESC_STATE_NONE;
        }else if(c == 0x43){    // C    Cursor Right
            charInEsc = ESC_STATE_NONE;
        }else if(c == 0x44){    // D    Cursor Left
            charInEsc = ESC_STATE_NONE;
        }else if(c == 0x30){    // 0    Response: terminal is OK 
        }else if(c == 0x6e){    // n    Escape Closing
            charInEsc = ESC_STATE_NONE;
        }
        ret = true;
    }
    return ret;
}
void ArduinoOS::clearBuffer(char* ca,unsigned int l){
    for(unsigned int i{0};i<l;i++)ca[i]=0;
};
void ArduinoOS::terminalNl(){
    if(locked){
        p(textEnterPassword,false);
    }else{
        snprintf(charIOBuffer,LONG,"%s:/>",aos_name.c_str());o(charIOBuffer,false);
    };
    clearBuffer(charIOBuffer,LONG);
}
void ArduinoOS::terminalHandleHistory(bool u){
    o("\33[2K",false,true);o(0x0D,false,true);
    if(u){
        snprintf(charIOBuffer,LONG,"%s:/>%s",aos_name.c_str(),terminalHistory);o(charIOBuffer,false);
        strcpy(charIOBuffer,terminalHistory);
        charIOBufferPos=strlen(terminalHistory);
    }else{
        snprintf(charIOBuffer,LONG,"%s:/>",aos_name.c_str());o(charIOBuffer,false);
        charIOBufferPos=0;
        clearBuffer(charIOBuffer,LONG);
    }
}
void ArduinoOS::terminalParseCommand(){
    uint8_t parCnt{0};
    char*   param[SHORT]{NULL};
    char*   split = strtok(charIOBuffer, " ");
    while(split){param[parCnt++] = split;split = strtok(0, " ");}
    if(parCnt>0) if(!callCommand(param[0],param,parCnt)) p(textCommandNotFound);
};
