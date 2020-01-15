#include <arduinoOS.h>
ArduinoOS aos;
String aos_date{__DATE__ " " __TIME__};
String aos_date_temp = aos_date;
String aos_name{"arduinoOS"};

//Global
void ArduinoOS::begin(HardwareSerial& Serial,unsigned int baud){
    wdt_enable(WDTO_4S);
    arduinoOS_default.begin();
    //Begin
    _begin      = true;
    _Serial     = &Serial;
    _Serial->begin(baud);
    #if defined ESP8266 || defined ESP32 
        EEPROM.begin(EEPROM_SIZE);
    #endif
    loadVariables();
    terminalWelcome();
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
};

//Commands
bool ArduinoOS::addCommand(char* name,void (*function)(char** param, uint8_t parCnt),char* description,bool hidden){
    AOS_CMD* b = new AOS_CMD{name,function,description,hidden,nullptr};
    if(aos_cmd == nullptr){
        aos_cmd = b;
    }else{
        AOS_CMD* i{aos_cmd};
        while(i->aos_cmd != nullptr){
            if(strcmp(i->name,name)==0) return false;
            i = i->aos_cmd;
        };
        i->aos_cmd = b;
    }
};
void ArduinoOS::listCommands(){
    o("Commands:");
    AOS_CMD* i{aos_cmd};
    while(i != nullptr){
        if(!i->hidden){
            snprintf(charOutBuffer,LONG, "%20s : %s",i->name,i->description);o(charOutBuffer);
        }
        i = i->aos_cmd;
    };
}
void ArduinoOS::manCommand(char* name){
    AOS_CMD* i{aos_cmd};
    while(i != nullptr){
        if(strcmp(i->name,name)==0){
            o(i->description);
            return true;
        }
        i = i->aos_cmd;
    };
    return false;
}
bool ArduinoOS::callCommand(char* name,char** param, uint8_t parCnt){
    AOS_CMD* i{aos_cmd};
    while(i != nullptr){
        if(strcmp(i->name,name)==0){
            (*(i->function))(param, parCnt);
            return true;
        }
        i = i->aos_cmd;
    };
    return false;
}

//Variables
bool ArduinoOS::addVariable(char* n,int& v,char* d,bool h,bool p)   {return _addVariable(n,&v,d,h,p,AOS_DT_INT);};
bool ArduinoOS::addVariable(char* n,double& v,char* d,bool h,bool p){return _addVariable(n,&v,d,h,p,AOS_DT_DOUBLE);};
bool ArduinoOS::addVariable(char* n,String& v,char* d,bool h,bool p){return _addVariable(n,&v,d,h,p,AOS_DT_STRING);};
bool ArduinoOS::_addVariable(char* name,void* value,char* description,bool hidden,bool protect,AOS_DT aos_dt){
    //if(_begin){o(textErrorBegin);return false;}
    AOS_VAR* b = new AOS_VAR{name,value,description,hidden,protect,aos_dt,nullptr};
    if(aos_var == nullptr){
        aos_var = b;
    }else{
        AOS_VAR* i{aos_var};
        while(i->aos_var != nullptr){
            if(strcmp(i->name,name)==0) return false;
            i = i->aos_var;
        };
        i->aos_var = b;
    } 
};
void ArduinoOS::listVariables(){
    o("Variables:");
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(!i->hidden){ 
            if(i->aos_dt==AOS_DT_INT)    snprintf(charOutBuffer,LONG,"%20s : %12d\t\t%s %s", i->name,*(int*)(i->value),i->description,(i->protect ? "(Protected)":""));
            if(i->aos_dt==AOS_DT_DOUBLE) {char str_temp[SHORT];dtostrf(*(double*)(i->value), 4, 2, str_temp);snprintf(charOutBuffer,LONG,"%20s : %12s\t\t%s %s", i->name,str_temp,i->description,(i->protect ? "(Protected)":""));};
            if(i->aos_dt==AOS_DT_STRING) snprintf(charOutBuffer,LONG,"%20s : %12s\t\t%s %s", i->name,(*(String*)(i->value)).c_str(),i->description,(i->protect ? "(Protected)":""));
            o(charOutBuffer);
        }
        i = i->aos_var;
    };
}
bool ArduinoOS::setVariable(char* name,char* value){
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(strcmp(i->name,name)==0){
            if(i->protect) return false;
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
        if(strcmp(i->name,name)==0){
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
                    EEPROM.write(p,NULL);p++;
                }else{
                    *(String*)(i->value) = "";
                    while(true){
                        char b = EEPROM.read(p);p++;
                        if((uint8_t)b == NULL) break;
                        *(String*)(i->value)+=b; 
                        if((*(String*)(i->value)).length() >= LONG) break;
                    }
                }
            }
            if(aos_date != aos_date_temp && !save){
                o("RESET");aos_date=aos_date_temp;
                loadVariables(true);return;
            }
        i = i->aos_var;
    };
    _usedEeprom = p;
    #if defined ESP8266 || defined ESP32 
        if(save) EEPROM.commit();
    #endif
};

//Interface
void ArduinoOS::o(char c,bool nl, bool esc){
    if(nl)  _Serial->println(c);    else    _Serial->print(c);
};
void ArduinoOS::o(char* ca,bool nl, bool esc){
    if(nl)  _Serial->println(ca);   else    _Serial->print(ca);
};
void ArduinoOS::cl(){
    o("\33[2K",false,true);o(0x0D,false,true); //Clear Line
}
void ArduinoOS::charIn(char c){
        if(charEsc(c)) return;
        if(c == 0x7F || c == 0x08){                                     //DEL BACKSPACE
            if(charInBufferPos > 0){
                o(0x08,false,true);                                     //BACKSPACE
                o(0x20,false,true);                                     //SPACE
                o(0x08,false,true);                                     //BACKSPACE
                charInBuffer[--charInBufferPos] = NULL;
            }else{
                o(0x07,false,true);                                     //BELL
            }
        }else if(c == 0x09){                                            //TAB
        }
        else if(c != 0x0A && c != 0x0D && charInBufferPos < LONG-1){    //NL+CR
            if(_SerialEcho) o(c,false);
            charInBuffer[charInBufferPos++] = c;
        }else{
            if(c == 0x0A && charInBufferLast == 0x0D || c == 0x0D && charInBufferLast == 0x0A) return;
            o(0x0A,false,true);o(0x0D,false,true);                      //NL+CR
            if(charInBufferPos>0){
                strcpy(terminalHistory,charInBuffer);
                terminalParseCommand();
                charInBufferPos=0;
            }
            clearBuffer(charInBuffer,LONG);
            snprintf(charOutBuffer,LONG,"%s:/>",aos_name.c_str());o(charOutBuffer,false);
        }
    charInBufferLast = c;
};
bool ArduinoOS::charEsc(char c){
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
            terminalConnectedIdle = true;
        }else if(c == 0x6e){    // n    Escape Closing
            charInEsc = ESC_STATE_NONE;
        }
        ret = true;
    }
    return ret;
}
void ArduinoOS::clearBuffer(char* ca,unsigned int l){
    for(unsigned int i{0};i<l;i++)ca[i]=NULL;
};
void ArduinoOS::terminalWelcome(){
    o(0x07,false,true);aos.o("\033[2J\033[1;1H");o(textWelcome);
    aos.listCommands();
    snprintf(charOutBuffer,LONG,"%s:/>",aos_name.c_str());o(charOutBuffer,false);
}
void ArduinoOS::terminalHandleHistory(bool u){
    cl();
    if(u){
        snprintf(charOutBuffer,LONG,"%s:/>%s",aos_name.c_str(),terminalHistory);o(charOutBuffer,false);
        strcpy(charInBuffer,terminalHistory);
        charInBufferPos=strlen(terminalHistory);
    }else{
        snprintf(charOutBuffer,LONG,"%s:/>",aos_name.c_str());o(charOutBuffer,false);
        clearBuffer(charInBuffer,LONG);
        charInBufferPos=0;
    }
}
void ArduinoOS::terminalParseCommand(){
    uint8_t parCnt{0};
    char*   param[SHORT];
    char*   split = strtok(charInBuffer, " ");
    while(split){param[parCnt++] = split;split = strtok(NULL, " ");}
    if(parCnt>0) if(!callCommand(param[0],param,parCnt)) o(textCommandNotFound);
};
