#pragma once

//
//  arduinoOS.h
//  V1.1
//
//  Created by Calvin Köcher on 15.06.20.
//  Copyright © 2020 Calvin Köcher. All rights reserved.
//  https://github.com/calkoe/arduinoOS
//

#include <arduino.h>
#include <EEPROM.h>

#if defined ESP8266
    #define SHORT               128     //Programm Parameter, Parameter Count
    #define LONG                128     //BufferIn, BufferOut, TerminalHistory
    #define SERSPEED            115200 
    #define STATUSLED           16
    #define EEPROM_SIZE         1024    //Only for ESP
#else
    #include <avr/wdt.h>
    #define SHORT               16      //Programm Parameter, Parameter Count
    #define LONG                64      //IO Buffer
    #define SERSPEED            9600 
    #define STATUSLED           LED_BUILTIN
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
const char textEscClear[] PROGMEM           = "\033[2J\033[1;1H";
const char textOk[] PROGMEM                 = "ok";

class ArduinoOS{
    
    private:

        //Global
        enum AOS_DT    { AOS_DT_BOOL, AOS_DT_INT, AOS_DT_DOUBLE, AOS_DT_STRING };
        enum AOS_ESC   { ESC_STATE_NONE, ESC_STATE_START, ESC_STATE_CODE};
        struct AOS_CMD {
            char*       name;
            void        (*function)(char**, u8);
            char*       description;
            bool        hidden;
            AOS_CMD*    aos_cmd;
        };
        struct AOS_VAR {
            char*       name;
            void*       value;
            char*       description;
            bool        hidden;
            bool        protect;
            AOS_DT      aos_dt;
            AOS_VAR*    aos_var;
        };
        struct AOS_EVT {
            char*       name;
            void        (*function)(void*);
            bool        active;
            void*       value;
            AOS_EVT*    aos_evt;
        };
        struct AOS_TASK {
            char*       name;
            void        (*function)();
            bool        active;
            u64         timestamp;
            u16         interval;
            bool        single;
            AOS_EVT*    aos_task;
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

        //Interface
        static void aos_gpio(char**,u8);
        static void aos_help(char**,u8);
        static void aos_load(char**,u8);
        static void aos_save(char**,u8);
        static void aos_get(char**,u8);
        static void aos_set(char**,u8);
        static void aos_stats(char**,u8);
        static void aos_clear(char**,u8);
        static void aos_lock(char**,u8);
        static void aos_reboot(char**,u8);
        static void aos_reset(char**,u8);

        //IO
        static void    charIn(char,bool);
        static bool    charEsc(char);
        static void    terminalPrefix();
        static void    terminalLine();
        static void    clearBuffer(char*,unsigned int);
        static bool    _variableAdd(char*,void*,char*,bool,bool,AOS_DT);
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
        static u8               statusLed;
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


        //API Events
        static void             eventListen(char*,void(*)(void*));
        static void             eventEmit(char*,void*,bool = false);
        static void             eventLoop();

        //API Commands
        static bool             commandAdd(char*,void (*)(char**, u8),char* = "",bool = false);
        static void             commandList(char* = "");
        static void             commandMan(char*);
        static bool             commandCall(char*,char** = 0, u8 = 0);

        //API Variables
        static bool             variableAdd(char*,bool&,  char* = "",bool = false,bool = false);
        static bool             variableAdd(char*,int&,   char* = "",bool = false,bool = false);
        static bool             variableAdd(char*,double&,char* = "",bool = false,bool = false);
        static bool             variableAdd(char*,String&,char* = "",bool = false,bool = false);
        static void             variableList(char* = "");
        static bool             variableSet(char*,char*);
        static void*            variableGet(char*);
        static void             variableLoad(bool = false);

        //API IO
        static void    o(const char,bool=true);
        static void    o(const char*,bool=true);
        static void    o(String,bool=true);
        static void    p(const char*,bool=true);

};