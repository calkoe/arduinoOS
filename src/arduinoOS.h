#pragma once

//
//  arduinoOS.h
//  V1.1
//
//  Created by Calvin Köcher on 15.06.20.
//  Copyright © 2020 Calvin Köcher. All rights reserved.
//  https://github.com/calkoe/arduinoOS
//

#include <EEPROM.h>

#if defined ESP8266
    #include <ESP.h>
    #define SHORT               128     //Programm Parameter, Parameter Count
    #define LONG                128     //BufferIn, BufferOut, TerminalHistory
    #define SERSPEED            115200 
    #define STATUSLED           16
    #define RESETBUTTON         0
    #define EEPROM_SIZE         1024    //Only for ESP
#else
    #include <Arduino.h>
    #include <avr/wdt.h>
    #define SHORT               16      //Programm Parameter, Parameter Count
    #define LONG                64      //IO Buffer
    #define SERSPEED            9600 
    #define STATUSLED           LED_BUILTIN
    #define RESETBUTTON         0
#endif

//Text
const char textErrorBegin[] PROGMEM         = "call variableAdd() before begin() !";
const char textWelcome[] PROGMEM            = "ArduinOS - https://github.com/calkoe/arduinoOS";
const char textCommandNotFound[] PROGMEM    = "Command not found! Try 'help' for more information.";
const char textInvalidParameter[] PROGMEM   = "Invalid parameter!";
const char textEnterPassword[] PROGMEM      = "Enter password to login: ";
const char textCommands[] PROGMEM           = "Commands:";
const char textVariables[] PROGMEM          = "Variables:";
const char textNotFound[] PROGMEM           = "Parameter not found!";
const char textTasks[] PROGMEM              = "Current Tasks";
const char textEscClear[] PROGMEM           = "\033[2J\033[1;1H";
const char textOk[] PROGMEM                 = "ok";

class ArduinoOS{
    
    private:

        //Global
        enum AOS_DT    { AOS_DT_BOOL, AOS_DT_INT, AOS_DT_DOUBLE, AOS_DT_STRING };
        enum AOS_ESC   { ESC_STATE_NONE, ESC_STATE_START, ESC_STATE_CODE};
        struct AOS_CMD {
            const char* name;
            void        (*function)(char**, u8);
            const char* description;
            bool        hidden;
            AOS_CMD*    aos_cmd;
        };
        struct AOS_VAR {
            const char* name;
            void*       value;
            const char* description;
            bool        hidden;
            bool        protect;
            AOS_DT      aos_dt;
            AOS_VAR*    aos_var;
        };
        struct AOS_EVT {
            const char* name;
            void        (*function)(void*);
            bool        active;
            void*       value;
            AOS_EVT*    aos_evt;
        };
        struct AOS_TASK {
            u16                 id;
            void                (*function)();
            u64                timestamp;
            u16                 interval;
            const char*         description;  
            u16                 time;  
            u16                 timeMax;  
            bool                repeat;
            AOS_TASK*           aos_task;
        };
        static unsigned         usedEeprom;
        static AOS_CMD*         aos_cmd;
        static AOS_VAR*         aos_var;
        static AOS_EVT*         aos_evt;
        static AOS_TASK*        aos_task;


    protected:

        //Global
        static bool             isBegin;
        static u8               IOC;
        static unsigned         IOP[2];
        static char             IO[2][LONG];
        static char             OUT[LONG];
        static u32              loopCounter;

        //IO
        static void    charIn(char,bool);
        static bool    charEsc(char);
        static void    clearBuffer();
        static bool    _variableAdd(const char*,void*,const char*,bool,bool,AOS_DT);
        static void    terminalPrefix();
        static void    terminalLine();
        static void    terminalParseCommand();


    public:   

        //Global
        ArduinoOS();
        static void             begin();
        static void             loop();
        static int              freeMemory();

        //API Settings
        static bool             serialEnable;
        static HardwareSerial*  serialInstance;
        static u32              serialBaud;
        static u8               status;
        static s8               statusLed;
        static s8               resetButton;
        static bool             watchdogEnable;
        static bool             autoLoad;
        static bool             autoReset;
        static bool             locked;
        static String           date;
        static String           date_temp;
        static String           hostname;
        static String           password;
        static String           firmware;

        //API TASKS
        static u16              setInterval(void(*)(),u16,const char* = "");
        static u16              setTimeout(void(*)(),u16,const char* = "",bool = false);
        static AOS_TASK*        unsetInterval(u16);
        static void             taskLoop();
        static void             taskManager();

        //API Events
        static void             eventListen(const char*,void(*)(void*));
        static void             eventEmit(const char*,void*,bool = false);
        static void             eventLoop();

        //API Commands
        static bool             commandAdd(const char*,void (*)(char**, u8),const char* = "",bool = false);
        static void             commandList(const char* = "");
        static void             commandMan(const char*);
        static bool             commandCall(const char*,char** = 0, u8 = 0);

        //API Variables
        static bool             variableAdd(const char*,bool&,  const char* = "",bool = false,bool = false);
        static bool             variableAdd(const char*,int&,   const char* = "",bool = false,bool = false);
        static bool             variableAdd(const char*,double&,const char* = "",bool = false,bool = false);
        static bool             variableAdd(const char*,String&,const char* = "",bool = false,bool = false);
        static void             variableList(const char* = "");
        static bool             variableSet(const char*,char*);
        static void*            variableGet(const char*);
        static void             variableLoad(bool = false);

        //API IO
        static void    o(const char,bool=true);
        static void    o(const char*,bool=true);
        static void    o(String,bool=true);
        static void    p(const char*,bool=true);

};