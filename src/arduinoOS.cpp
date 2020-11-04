#include <arduinoOS.h>

//Global
bool                    ArduinoOS::isBegin = false;
HardwareSerial*         ArduinoOS::serialInstance{&Serial};
ArduinoOS::AOS_CMD*     ArduinoOS::aos_cmd{nullptr};
ArduinoOS::AOS_VAR*     ArduinoOS::aos_var{nullptr};
ArduinoOS::AOS_EVT*     ArduinoOS::aos_evt{nullptr};
ArduinoOS::AOS_TASK*    ArduinoOS::aos_task{nullptr};
u8                      ArduinoOS::IOC{0};
unsigned                ArduinoOS::IOP[]{0};
char                    ArduinoOS::IO[][LONG]{0};
char                    ArduinoOS::OUT[LONG]{0};
u8                      ArduinoOS::status{5};
unsigned                ArduinoOS::usedEeprom{0};
bool                    ArduinoOS::watchdogEnable{false};
bool                    ArduinoOS::serialEnable{true};
u32                     ArduinoOS::serialBaud{SERSPEED};
bool                    ArduinoOS::autoLoad{true};
bool                    ArduinoOS::autoReset{true};
bool                    ArduinoOS::locked{false};
u8                      ArduinoOS::statusLed{STATUSLED};
u8                      ArduinoOS::resetButton{RESETBUTTON};
String                  ArduinoOS::date{__DATE__ " " __TIME__};
String                  ArduinoOS::date_temp{date};
String                  ArduinoOS::hostname{"arduinoOS"};
String                  ArduinoOS::password{"aos"};
String                  ArduinoOS::firmware{"-"};

void ArduinoOS::begin(){
    //Status LED
    if(statusLed){
        setInterval([](){
            if(!status)          digitalWrite(statusLed,!digitalRead(statusLed));
            else if(status==5)   digitalWrite(statusLed,0);
            else{
                static bool o{1};
                static u8 p{0};
                if(p<2*status) digitalWrite(statusLed,o=!o);
                if(p==10){p=0;o=1;}else p++;
            }
        },180);
    };
    //LOOP 10ms
    setInterval([](){
        //Watchdog
        wdt_reset();
        //Read Serial
        while(serialEnable && serialInstance->available()){
            while(serialInstance->available())
                charIn(serialInstance->read(),true);
        }
    },10);
    //Setup
    isBegin = true;
    if(serialEnable) serialInstance->begin(serialBaud);
    if(watchdogEnable) wdt_enable(WDTO_4S);
    #if defined ESP8266 || defined ESP32 
        EEPROM.begin(EEPROM_SIZE);
    #endif
    if(statusLed)   pinMode(statusLed,OUTPUT);
    if(resetButton) pinMode(resetButton,INPUT_PULLUP);
    if(autoLoad)  variableLoad();
    o(0x07,false);
    p(textEscClear);
    p(textWelcome);
    o(firmware);
    o("");
    commandList();
    terminalPrefix();
    clearBuffer(IO[IOC],LONG);
};
void ArduinoOS::loop(){
    //Events
    eventLoop();
    //Tasks
    taskLoop();
};

/**
 * Set Interval
 * 
 * @param  function Function Callback
 * @param  intervall
 * @return intervall Id
*/
u16 ArduinoOS::setInterval(void(*f)(),u16 i){
    return setTimeout(f,i,true);
};

/**
 * Set Timeout
 * 
 * @param  function Function Callback
 * @param  intervall 
 * @param  repeat 
 * @return timeout Id
*/
u16 ArduinoOS::setTimeout(void(*f)(),u16 i, bool repeat){
    static u16 idSerial{1};
    AOS_TASK* e = new AOS_TASK{idSerial,f,millis(),i,repeat,nullptr};
    if(!aos_task){
        aos_task = e;
    }else{
        AOS_TASK* i{aos_task};
        while(i->aos_task) i = i->aos_task;
        i->aos_task = e;
    }
    return idSerial++;
};

/**
 * Unset Interval
 * 
 * @param idDel IntervalID
*/
ArduinoOS::AOS_TASK* ArduinoOS::unsetInterval(u16 idDel){
    if(!aos_task) return nullptr;
    if(aos_task->id == idDel){
        AOS_TASK* d{aos_task}; 
        aos_task = aos_task->aos_task;
        delete d;
        return nullptr;
    }
    AOS_TASK* i{aos_task};
    while(i->aos_task){
        if(i->aos_task->id == idDel){
            AOS_TASK* d{i->aos_task}; 
            i->aos_task = i->aos_task->aos_task;
            delete d;
            return i->aos_task;
        }
        i = i->aos_task;
    }
    return nullptr;
};

/**
 * Manage Task Loop
*/
void ArduinoOS::taskLoop(){
    AOS_TASK* i{aos_task};
    while(i){
        if(i->interval && (unsigned long)(millis()-i->timestamp)>=i->interval){
            //o("Abweichung: " + (String)(unsigned long)(ms-i->timestamp));
            (*(i->function))();
            i->timestamp = millis();
            i = !i->repeat ? unsetInterval(i->id) : i->aos_task;
        }else i = i->aos_task;
    }
};

/**
 * Register new Event-Listener
 * 
 * @param name Event name
 * @param function void (*function)(void*)
*/
void ArduinoOS::eventListen(const char* name,void (*function)(void*)){
    AOS_EVT* e = new AOS_EVT{name,function,false,nullptr,nullptr};
    if(aos_evt == nullptr){
        aos_evt = e;
    }else{
        AOS_EVT* i{aos_evt};
        while(i->aos_evt != nullptr) i = i->aos_evt;
        i->aos_evt = e;
    }
};

/**
 * Emit event
 * 
 * @param name Event Name
 * @param value Pass Value to listener
 * @param now Call Listener immediately or on next loop
*/
void ArduinoOS::eventEmit(const char* name,void* value,bool now){
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

/**
 * Manage Event Loop
*/
void ArduinoOS::eventLoop(){
    AOS_EVT* i{aos_evt};
    while(i != nullptr){
        if(i->active){
            (*(i->function))(i->value);
            i->active = false;
        }
        i = i->aos_evt;
    };
};

/**
 * Adds a new Command to CLI
 * 
 * @param name Command Name
 * @param function void (*function)(char** param, u8 parCnt)
 * @param description Command description
 * @param hidden Hide Command in CLI
 * @return bool true if Command added successfully
*/
bool ArduinoOS::commandAdd(const char* name,void (*function)(char** param, u8 parCnt),const char* description,bool hidden){
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

/**
 * List aviable Commands on CLI
 * 
 * @param filter Filter on Command name
*/
void ArduinoOS::commandList(const char* filter){
    p(textCommands);
    AOS_CMD* i{aos_cmd};
    while(i != nullptr){
        if(!i->hidden && (!filter || strstr(i->name, filter))){
            snprintf(OUT,LONG, "%-20s %s",i->name,i->description);o(OUT);
        }
        i = i->aos_cmd;
    };
}

/**
 * Display Command manual on CLI
 * 
 * @param filter Filter by Command name
*/
void ArduinoOS::commandMan(const char* name){
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

/**
 * Run Command with Parameter
 * 
 * @param name Name of the Command
 * @param param Parameter
 * @param paramCnt Parameter count
*/
bool ArduinoOS::commandCall(const char* name,char** param, u8 parCnt){
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
bool ArduinoOS::variableAdd(const char* n,bool& v,const char* d,bool h,bool p)  {return _variableAdd(n,&v,d,h,p,AOS_DT_BOOL);};
bool ArduinoOS::variableAdd(const char* n,int& v,const char* d,bool h,bool p)   {return _variableAdd(n,&v,d,h,p,AOS_DT_INT);};
bool ArduinoOS::variableAdd(const char* n,double& v,const char* d,bool h,bool p){return _variableAdd(n,&v,d,h,p,AOS_DT_DOUBLE);};
bool ArduinoOS::variableAdd(const char* n,String& v,const char* d,bool h,bool p){return _variableAdd(n,&v,d,h,p,AOS_DT_STRING);};
bool ArduinoOS::_variableAdd(const char* name,void* value,const char* description,bool hidden,bool protect,AOS_DT aos_dt){
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
void ArduinoOS::variableList(const char* filter){
    p(textVariables);
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(!i->hidden && (!filter || strstr(i->name, filter))){ 
            if(i->aos_dt==AOS_DT_BOOL)   snprintf(OUT,LONG,"%-20s : %-20s\t%s %s", i->name,*(bool*)(i->value) ? "true" : "false",i->description,(i->protect ? "(Protected)":""));
            if(i->aos_dt==AOS_DT_INT)    snprintf(OUT,LONG,"%-20s : %-20d\t%s %s", i->name,*(int*)(i->value),i->description,(i->protect ? "(Protected)":""));
            if(i->aos_dt==AOS_DT_DOUBLE) {char str_temp[SHORT];dtostrf(*(double*)(i->value), 4, 2, str_temp);snprintf(OUT,LONG,"%-20s : %-20s\t%s %s", i->name,str_temp,i->description,(i->protect ? "(Protected)":""));};
            if(i->aos_dt==AOS_DT_STRING) snprintf(OUT,LONG,"%-20s : %-20s\t%s %s", i->name,(*(String*)(i->value)).c_str(),i->description,(i->protect ? "(Protected)":""));
            o(OUT);
        }
        i = i->aos_var;
    };
}
bool ArduinoOS::variableSet(const char* name,char* value){
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
void* ArduinoOS::variableGet(const char* name){
    AOS_VAR* i{aos_var};
    while(i != nullptr){
        if(!strcmp(i->name,name)) return i->value;
        i = i->aos_var;
    };
    return nullptr;
};
void ArduinoOS::variableLoad(bool save){
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
                        if((u8)b == (char)0) break;
                        *(String*)(i->value)+=b; 
                        if((*(String*)(i->value)).length() >= LONG) break;
                    }
                }
            }
            if(((date != date_temp && autoReset) || date == "!") && !save){
                o("RESET");date=date_temp;
                variableLoad(true);return;
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
        if(serialEnable) serialInstance->print(ca);
        eventEmit("o",(void*)ca,true);
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
        if(charEsc(c)) return;                                             //Filter ESC 
        if(c != 13 && c != 10 && c != 8 && c != 127 && echo) o(c,false);   //No Echo CR or NL or Backspace or DEL 
        if(c == 8 || c == 127){                                            //BACKSPACE or DEL
            if(IOP[IOC] > 0){
                IO[IOC][--IOP[IOC]] = 0;
                o("\b \b",false);
            }
        }else if(c != 13 && c != 10 && IOP[IOC] < LONG-1){                  //CR NL
            IO[IOC][IOP[IOC]++] = c;                    
        }else if(!(lc == 13 && c == 10)){                                   //CR NL 
            o(13,false);o(10,false);                                        //CR NL
            if(IOP[IOC]>0 && !locked) terminalParseCommand();
            if(!strcmp(IO[IOC],password.c_str())) locked = false;
            IOC = !IOC;
            terminalPrefix();
            clearBuffer(IO[IOC],LONG);
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
        };
        charInEsc = ESC_STATE_NONE;
        ret = true;
    }
    return ret;
}
void ArduinoOS::clearBuffer(char* ca,unsigned int l){
    for(unsigned int i{0};i<l;i++)ca[i]=0;IOP[IOC]=0;
};
void ArduinoOS::terminalPrefix(){
    if(locked){
        p(textEnterPassword,false);
    }else{
        snprintf(OUT,LONG,"%s:/>",hostname.c_str());o(OUT,false);
    };
}
void ArduinoOS::terminalLine(){
    o("\r",false);
    for(u8 i{0};i<LONG;i++) o(32,false);
    o("\r",false);
    terminalPrefix();
    o(IO[IOC],false);
}
void ArduinoOS::terminalParseCommand(){
    u8     parCnt{0};
    char*       param[SHORT]{NULL};
    char        search{' '};
    unsigned    s{0};
    for(unsigned i{0};i<LONG;i++){
        if(IO[IOC][i] == 0) break;
        while((IO[IOC][i] == '"' || IO[IOC][i] == ' ') && IO[IOC][i] != 0) search = IO[IOC][i++];
        s = i;
        while(IO[IOC][i] != search && IO[IOC][i] != 0) i++;
        char* buffer = (char*)malloc(i-s+1);
        for(unsigned j{0};j<(i-s);j++) buffer[j] = IO[IOC][s+j];
        buffer[i-s] = 0;param[parCnt++] = buffer;
    }
    if(parCnt>0) if(!commandCall
    (param[0],param,parCnt)) p(textCommandNotFound);
    for(u8 i{0};i<parCnt;i++) delete param[i];
};

//Interface Methods
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

ArduinoOS::ArduinoOS(){
    variableAdd("sys/date",     date,"",true,true);
    variableAdd("sys/hostname", hostname,"");
    variableAdd("sys/password", password,"");

    commandAdd("gpio",[](char** param,u8 parCnt){
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
        commandMan("gpio");
        return;
        }
    },"🖥  gpio [pin] [0|1]");

    commandAdd("help",[](char** param,u8 parCnt){
        if(parCnt == 1)
        commandList();
        if(parCnt == 2)
        commandList(param[1]);
    },"",true);

    commandAdd("get",[](char** param,u8 parCnt){
        if(parCnt == 2)
            variableList(param[1]);
        else
            variableList();
    },"🖥  get [filter]");

    commandAdd("set",[](char** param,u8 parCnt){
        if(parCnt < 2 || parCnt > 3){
            p(textInvalidParameter);
            commandMan("set");
            return;
        }
        if(variableSet(param[1],param[2])){
        variableLoad(true);
        p(textOk);
        }else{
        p(textNotFound); 
        } 
    },"🖥  set [par] [val]");

    commandAdd("clear",[](char** param,u8 parCnt){
        p(textEscClear);
    },"",true);

    commandAdd("lock",[](char** param,u8 parCnt){
        locked = true;
    },"🖥");

    commandAdd("reboot",[](char** param,u8 parCnt){
        #if defined ESP8266
            ESP.restart();
        #endif
        for(int i{0};;i++){o('.',false);delay(333);}
    },"🖥");

    commandAdd("reset",[](char** param,u8 parCnt){
        date = "!";
        variableLoad(true);
        commandCall("reboot");
    },"🖥");

    commandAdd("status", [](char** param,u8 parCnt){
        o("🖥  System:");
        snprintf(OUT,LONG,"%-20s : %s","FIRMWARE",firmware.c_str());o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","COMPILED",date.c_str());o(OUT);
        snprintf(OUT,LONG,"%-20s : %d B","HEAP",freeMemory());o(OUT);
        snprintf(OUT,LONG,"%-20s : %d B","EERPOM",usedEeprom);o(OUT);
    },"🖥");

}