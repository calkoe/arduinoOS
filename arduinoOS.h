//
//  arduinoOS.h
//  V1
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
    #define SHORT               32      //Programm Parameter, Parameter Count
    #define LONG                64      //BufferIn, BufferOut, TerminalHistory
#endif

//Plugins
#include <arduinoOS_default.h>

//Text
const char textErrorBegin[] PROGMEM         = "call addVariable() before begin()";
const char textWelcome[] PROGMEM            = "ArduinOS V1.0 - https://github.com/calkoe/arduinoOS\r\n\r\n";
const char textCommandNotFound[] PROGMEM    = "Command not found! Try 'help' for more information.";
const char textInvalidParameter[] PROGMEM   = "Invalid parameter!";
const char textEnterPassword[] PROGMEM      = "Please enter password: ";
const char textCommands[] PROGMEM           = "Commands:";
const char textVariables[] PROGMEM          = "Variables:";
const char textNotFound[] PROGMEM           = "Parameter not found!";
const char textEscClear[] PROGMEM           = "\033[2J\033[1;1H";

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

        bool            isBegin{false};
        HardwareSerial* serialInstance;
        AOS_CMD*        aos_cmd{nullptr};
        AOS_VAR*        aos_var{nullptr};

        char            charInBuffer[LONG];
        unsigned int    charInBufferPos{0};
        char            charOutBuffer[LONG];
        char            terminalHistory[LONG];

        bool            _addVariable(char*,void*,char*,bool,bool,AOS_DT);
        
    public:   

        //Global
        ArduinoOS();
        void    begin(HardwareSerial&, unsigned int = 9600);
        void    loop();

        //Settings
        unsigned int    usedEeprom{0};
        bool            serialEcho{true};
        bool            enableWatchdog{true};
        bool            enableSerial{true};
        bool            autoLoad{true};
        bool            autoReset{true};
        bool            locked{false};

        //Commands
        bool    addCommand(char*,void (*)(char**, uint8_t),char* = "",bool = false);
        void    listCommands(char* = "");
        void    manCommand(char*);
        bool    callCommand(char*,char** = NULL, uint8_t = 0);

        //Variables
        bool    addVariable(char*,bool&,  char* = "",bool = false,bool = false);
        bool    addVariable(char*,int&,   char* = "",bool = false,bool = false);
        bool    addVariable(char*,double&,char* = "",bool = false,bool = false);
        bool    addVariable(char*,String&,char* = "",bool = false,bool = false);
        void    listVariables(char* = "");
        bool    setVariable(char*,char*);
        bool    getVariable(char*,char*);
        void    loadVariables(bool = false);

        //Interface
        void    o(const char,bool=true,bool=false);
        void    o(const char*,bool=true,bool=false);
        void    p(const char*,bool=true,bool=false);

        void    charIn(char);
        bool    charEsc(char);
        void    clearBuffer(char*,unsigned int);
        void    terminalNl();
        void    terminalHandleHistory(bool);
        void    terminalParseCommand();

};
extern ArduinoOS aos;
extern String aos_date;
extern String aos_name;
extern String aos_password;