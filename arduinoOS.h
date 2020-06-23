//
//  arduinoOS.h
//  V1.1
//
//  Created by Calvin Köcher on 15.06.20.
//  Copyright © 2020 Calvin Köcher. All rights reserved.
//  https://github.com/calkoe/arduinoOS
//

#pragma once
#include <arduino.h>
#include <EEPROM.h>

#if defined ESP8266
    #define SHORT               128     //Programm Parameter, Parameter Count
    #define LONG                128     //BufferIn, BufferOut, TerminalHistory
    #define SERSPEED            112500 
    #define STATUSLED           16
    #define EEPROM_SIZE         1024    //Only for ESP
#else
    #include <avr/wdt.h>
    #define SHORT               16      //Programm Parameter, Parameter Count
    #define LONG                64      //IO Buffer
    #define SERSPEED            9600 
    #define STATUSLED           0
#endif


//Text
const char textErrorBegin[] PROGMEM         = "call addVariable() before begin() !";
const char textWelcome[] PROGMEM            = "ArduinOS V1.1 - https://github.com/calkoe/arduinoOS\r\n";
const char textCommandNotFound[] PROGMEM    = "Command not found! Try 'help' for more information.";
const char textInvalidParameter[] PROGMEM   = "Invalid parameter!";
const char textEnterPassword[] PROGMEM      = "Please enter password: ";
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
            void        (*function)(char**, uint8_t);
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
        static bool             isBegin;
        static unsigned         usedEeprom;
        static AOS_CMD*         aos_cmd;
        static AOS_VAR*         aos_var;
        static AOS_EVT*         aos_evt;

        //IO
        static unsigned charIOBufferPos;
        static char    terminalHistory[LONG];
        static bool    _addVariable(char*,void*,char*,bool,bool,AOS_DT);
        static void    clearBuffer(char*,unsigned int);
        static void    terminalHandleHistory(bool);
        static void    terminalParseCommand();
        
    protected:

        //Global
        static char             charIOBuffer[LONG];

        //Interface
        static int  freeMemory();
        static void aos_gpio(char**,uint8_t);
        static void aos_help(char**,uint8_t);
        static void aos_load(char**,uint8_t);
        static void aos_save(char**,uint8_t);
        static void aos_get(char**,uint8_t);
        static void aos_set(char**,uint8_t);
        static void aos_stats(char**,uint8_t);
        static void aos_clear(char**,uint8_t);
        static void aos_reboot(char**,uint8_t);
        static void aos_reset(char**,uint8_t);

        //IO
        static void    charIn(char,bool=true);
        static bool    charEsc(char);
        static void    terminalNl();

    public:   

        //Global
        ArduinoOS();
        static void             begin();
        static void             loop();

        //API Settings
        static bool             enableSerial;
        static HardwareSerial   serialInstance;
        static bool             serialEcho;
        static unsigned int     serialBaud;
        static uint8_t          status;
        static bool             enableWatchdog;
        static bool             autoLoad;
        static bool             autoReset;
        static bool             locked;
        static uint8_t          statusLed;
        static String           date;
        static String           date_temp;
        static String           hostname;
        static String           user;
        static String           password;

        //API Events
        static void             listenEvent(char*,void(*)(void*));
        static void             emitEvent(char*,void*,bool = false);
        static void             loopEvent();

        //API Commands
        static bool             addCommand(char*,void (*)(char**, uint8_t),char* = "",bool = false);
        static void             listCommands(char* = "");
        static void             manCommand(char*);
        static bool             callCommand(char*,char** = 0, uint8_t = 0);

        //API Variables
        static bool             addVariable(char*,bool&,  char* = "",bool = false,bool = false);
        static bool             addVariable(char*,int&,   char* = "",bool = false,bool = false);
        static bool             addVariable(char*,double&,char* = "",bool = false,bool = false);
        static bool             addVariable(char*,String&,char* = "",bool = false,bool = false);
        static void             listVariables(char* = "");
        static bool             setVariable(char*,char*);
        static void*            getValue(char*);
        static void             loadVariables(bool = false);

        //API IO
        static void    o(const char,bool=true);
        static void    o(const char*,bool=true);
        static void    o(String,bool=true);
        static void    p(const char*,bool=true);

        

};