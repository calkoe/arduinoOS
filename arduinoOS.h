#pragma once
#include <arduino.h>
#include <EEPROM.h>

#define SHORT               32          //Programm Parameter, Parameter Count
#define LONG                64          //BufferIn, BufferOut, Terminal History

class ArduinoOS{
    
    protected:

        enum AOS_DT    { AOS_DT_INT, AOS_DT_DOUBLE, AOS_DT_STRING };
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

        bool            _begin{false};
        HardwareSerial* _Serial;
        unsigned int    _SerialBaud{0};
        bool            _SerialEcho{true};
        AOS_CMD*        aos_cmd{nullptr};
        AOS_VAR*        aos_var{nullptr};

        char            charInBuffer[LONG];
        char            charInBufferLast;
        AOS_ESC         charInEsc{ESC_STATE_NONE};
        unsigned int    charInBufferPos{0};
        char            charOutBuffer[LONG];
        
        unsigned long   timer1{0};
        bool            terminalConnectedIdle = false;
        bool            terminalConnected     = false;
        char            terminalHistory[LONG];
        
        bool            _addVariable(char*,void*,char*,bool,bool,AOS_DT);
        
    public:

        //Storage
        char* textErrorBegin{"addVariable() must be called before begin()"};
        char* textWelcome{"Welcome to arduinOS V1.0"};
        char* textCommandNotFound{"Command not found! Try 'help' for more information."};
        char* textInvalidParameter{"Invalid Parameter!"};

        //Global
        void    begin(HardwareSerial&, unsigned int = 9600);
        void    loop();

        //Commands
        bool    addCommand(char*,void (*)(char**, uint8_t),char* = "",bool = false);
        void    listCommands();
        void    manCommand(char*);
        bool    callCommand(char*,char** = NULL, uint8_t = NULL);

        //Variables
        bool    addVariable(char*,int&,   char* = "",bool = false,bool = false);
        bool    addVariable(char*,double&,char* = "",bool = false,bool = false);
        bool    addVariable(char*,String&,char* = "",bool = false,bool = false);
        void    listVariables();
        bool    setVariable(char*,char*);
        bool    getVariable(char*,char*);
        void    loadVariables(bool = false);

        //Interface
        void    o(char,bool=true,bool=false);
        void    o(char*,bool=true,bool=false);
        void    cl();

        void    charIn(char);
        bool    charEsc(char);
        void    clearBuffer(char*,unsigned int);
        void    terminalWelcome();
        void    terminalHandleHistory(bool);
        void    terminalParseCommand();

};
extern ArduinoOS aos;
extern String aos_name;
#include <arduinoOS_default.h>