//
//  arduinoOS.h
//  V1.1
//
//  Created by Calvin Köcher on 09.01.20.
//  Copyright © 2020 Calvin Köcher. All rights reserved.
//  https://github.com/calkoe/arduinoOS
//

#pragma once
#include <arduino.h>
#include <EEPROM.h>

#if defined ESP8266 || defined ESP32 
    #define EEPROM_SIZE         1024    //Only for ESP
    #define SHORT               128     //Programm Parameter, Parameter Count
    #define LONG                128     //BufferIn, BufferOut, TerminalHistory
#else
    #include <avr/wdt.h>
    #define SHORT               16      //Programm Parameter, Parameter Count
    #define LONG                64      //IO Buffer
#endif


//Text
const char textErrorBegin[] PROGMEM         = "call addVariable() before begin() !";
const char textWelcome[] PROGMEM            = "ArduinOS V1.1 - https://github.com/calkoe/arduinoOS\r\n\r\n";
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
            void        (*function)(char);
            bool        active;
            char        payload;
            AOS_EVT*    aos_evt;
        };
        bool            isBegin{false};
        HardwareSerial* serialInstance;
        AOS_CMD*        aos_cmd{nullptr};
        AOS_VAR*        aos_var{nullptr};
        AOS_EVT*        aos_evt{nullptr};

        char            charIOBuffer[LONG];
        unsigned        charIOBufferPos{0};
        char            terminalHistory[LONG];

        bool            _addVariable(char*,void*,char*,bool,bool,AOS_DT);
        
    public:   

        //Global
        ArduinoOS();
        void    begin(HardwareSerial&, unsigned int = 9600);
        void    loop();

        //Settings
        unsigned    usedEeprom{0};
        bool        serialEcho{true};
        bool        enableWatchdog{true};
        bool        enableSerial{true};
        bool        autoLoad{true};
        bool        autoReset{true};
        bool        locked{false};
        String      aos_date{__DATE__ " " __TIME__};
        String      aos_date_temp = aos_date;
        String      aos_name{"root"};
        String      aos_password{"root"};

        //Events
        void    listenEvent(char*,void (*)(char));
        void    emitEvent(char*,char,bool = false);
        void    loopEvent();

        //Commands
        bool    addCommand(char*,void (*)(char**, uint8_t),char* = (char)0,bool = false);
        void    listCommands(char* = (char)0);
        void    manCommand(char*);
        bool    callCommand(char*,char** = (char)0, uint8_t = 0);

        //Variables
        bool    addVariable(char*,bool&,  char* = (char)0,bool = false,bool = false);
        bool    addVariable(char*,int&,   char* = (char)0,bool = false,bool = false);
        bool    addVariable(char*,double&,char* = (char)0,bool = false,bool = false);
        bool    addVariable(char*,String&,char* = (char)0,bool = false,bool = false);
        void    listVariables(char* = (char)0);
        bool    setVariable(char*,char*);
        bool    getVariable(char*,char*);
        void    loadVariables(bool = false);

        //Interface
        void    o(const char,bool=true,bool=false);
        void    o(const char*,bool=true,bool=false);
        void    o(String,bool=true,bool=false);
        void    p(const char*,bool=true,bool=false);

        void    charIn(char);
        bool    charEsc(char);
        void    clearBuffer(char*,unsigned int);
        void    terminalNl();
        void    terminalHandleHistory(bool);
        void    terminalParseCommand();

        //Default Commands
        void        defaultInit();
        int         freeMemory();
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

};
extern ArduinoOS aos;