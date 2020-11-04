<img src="img/arduinoOS.png" width="300"></img>

**A powerful library for saving and modify your application settings**

**Features:**
* UNIX style serial terminal
* EEPROM support for saving parameters
* Enables access to application-parameter and application-functions
* Robust design with no Heap-Fragmentation
* Integrated Eventmanager + Timer
* Wifi + Telnet + Mqtt + OTA Features (ESP8266 only)
* Small AVR footprint (<770 Bytes of SRAM)

**Serial CLI: (Baud 9600 on AVR | 112500 on ESP8266)**
```
arduinoOS:/>help
Commands:
gpio                 🖥  gpio [pin] [0|1]
get                  🖥  get [filter]
set                  🖥  set [par] [val]
lock                 🖥
reboot               🖥
reset                🖥
status               🖥
wifiStatus           📶 Shows System / Wifi status
wifiFirmware         📶 [url] | load and install new firmware from URL (http or https)
wifiScan             📶 Scans for nearby networks
wifiConnect          📶 [network] [password] | apply network settings and connect to configured network
wifiDns              📶 [ip] | check internet connection
mqttStatus           📡 Shows System / Wifi / MQTT status
mqttConnect          📡 [mqtt_server] [mqtt_port] [mqtt_user] [mqtt_password] | Apply and configured
mqttPublish          📡 [topic] [message] | publish a message to topic
arduinoOS:/>get
Variables:
sys/hostname         : arduinoOS                 
sys/password         : aos                       
wifi/enable          : false                    📶 Enable WiFi STA-Mode 
wifi/network         :                          📶 Network SSID 
wifi/password        :                          📶 Network Password 
wifi/ip              : 0.0.0.0                  📶 IP (leave blank to use DHCP) 
wifi/subnet          : 0.0.0.0                  📶 Subnet 
wifi/gateway         : 0.0.0.0                  📶 Gateway 
wifi/dns             : 0.0.0.0                  📶 DNS 
hotspot/enable       : false                    📶 Enable WiFi Hotspot-Mode 
hotspot/password     :                          📶 Hotspot Password 
telnet/enable        : true                     📶 Enable Telnet support on Port 23 (require reboot) 
ntp/enable           : false                    ⏱  Enable NTP Sync 
ntp/server           :                          ⏱  NTP Server adress 
ntp/offset           : 0                        ⏱  NTP Time offset 
mqtt/mqtt_enable     : false                    📡 mqtt_enable MQTT 
mqtt/mqtt_server     :                          📡 MQTT mqtt_server IP or Name 
mqtt/mqtt_port       : 1883                     📡 MQTT mqtt_server Port 
mqtt/mqtt_tls        : false                    📡 Use TLS 
mqtt/mqtt_tlsVerify  : false                    📡 Verify TLS Certificates 
mqtt/mqtt_user       :                          📡 Username 
mqtt/mqtt_password   :                          📡 Password 
demo/bool            : true                      
demo/int             : 1234                      
demo/double          : 1234.12                   
demo/string          : DEMO                     ..comment.. 
arduinoOS:/>
```
* EEPROM will automatically reset after sketch upload

<br/><br/>
<img src="img/memory.png" width="25" align="right"/>
**Parameter**
```cpp
    static bool             commandAdd(char* name,void (*)(char**, u8) callbackFunction,char* description,bool hidden);
    static bool             commandCall(char* name,char** param, u8 paramCnt);
```

<br/><br/>
**Commands**
<img src="img/commands.png" width="25" align="right"/>
```cpp
        static bool             variableAdd(char* name,bool&    variableRef,  char* description,bool hidden, bool protected);
        static bool             variableAdd(char* name,int&     variableRef,  char* description,bool hidden, bool protected);
        static bool             variableAdd(char* name,double&  variableRef,  char* description,bool hidden, bool protected);
        static bool             variableAdd(char* name,String&  variableRef,  char* description,bool hidden, bool protected);
```

<br/><br/>
**Events**
<img src="img/commands.png" width="25" align="right"/>
```cpp
        static void             eventListen(char* name,void(*)(void*) callbackFunction);
        static void             eventEmit(char* name,void* parameter,bool callImmediately);
```

<br/><br/>
**Tasks / Timeouts**
<img src="img/commands.png" width="25" align="right"/>
```cpp
        static u16              setInterval(void(*)() callbackFunction,u16 ms);
        static u16              setTimeout(void(*)() callbackFunction, u16 ms);
        static AOS_TASK*        unsetInterval(u16 id);
```

<br/><br/>
**ESP8266: MQTT**
<img src="img/commands.png" width="25" align="right"/>
```cpp
        static void publish(char* topic, char* payload, bool retain, u8 qos);
        static void publish(String& topic, String& payload, bool retain, u8 qos);
        static void subscripe(char* topic,u8,void (*function)(char*,char*) callbackFunction);
        static void unsubscripe(char* topic);
```

<br/><br/>
**Begin**
```cpp
aos.begin(Serial);
```
* **Serial**: Reference to serial interface to start the Terminal. Allowed data types: *HardwareSerial*.
* NOTICE: You have to add all your parameter before calling *aos.begin(Serial)* to make the EEPROM layout clear to the system. 
* NOTICE: Your command-function have to take these parameters: void(char** param,uint8_t parCnt){};

<br/><br/>
**AVR Example sketch:**
```cpp
#include <arduinoOS.h>
ArduinoOS aos

bool    demoBool{true};
int     demoInt{1234};
double  demoDouble{1234.1234};
String  demoString{"DEMO"};

void setup()
{
    aos.variableAdd("demo/bool",    demoBool);
    aos.variableAdd("demo/int",     demoInt);
    aos.variableAdd("demo/double",  demoDouble);
    aos.variableAdd("demo/string",  demoString, "..comment..", false, false);

    aos.commandAdd("demo",[](char** param,u8 parCnt){
        Serial.println("Hello from Demo Function");
        Serial.println("Parameter count: " + (String)parCnt);
        for(int i{0};i<parCnt;i++){
            Serial.println("- " + (String)param[i]);
        }
    });

    aos.setInterval([](){
        aos.o("Every Second");
    },1000);


    aos.setTimeout([](){
        aos.o("Once after 5 Seconds");
    },5000);

    aos.begin();
}

void loop()
{ 
    aos.loop();
}
```
<br/><br/>
**Roadmap:**
<img src="img/plugins.png" width="25" align="right"/>
- [x] Allow WIFI-Config on ESP8266
- [x] Telnet Server on ESP8266
- [x] Allow MQTT-Config on ESP8266 (TLS Supported)
- [x] Allow NTP-Config on ESP8266
- [x] HTTP OTA Update capability on ESP8266
- [ ] Add ESP32 Support
  
<br/><br/>
**Contact:**
* Calvin Köcher
* calvin.koecher@outlook.de
* https://www.imakeyouintelligent.com