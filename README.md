<img src="img/arduinoOS.png" width="300"></img>

**A powerful library for saving and modify your application settings**

**Features:**
* UNIX style serial terminal
* EEPROM support for saving parameters
* Enables access to application-parameter
* Enables access to application-functions
* Int, Double and String datatypes supported for parameter
* Robust design with no Heap-Fragmentation
* Small footprint (<770 Bytes of SRAM)

**Terminal:**
```
ArduinOS V1.0
Commands:
                demo : demo [parm1] [parm2] - ..comment.. 
                gpio : gpio [write|read] [#] [0|1]
                 get : get [parameter]
                 set : set [parameter] [value]
               stats : 
              reboot : 
               reset : 
arduinoOS:/>get
Variables:
            sys/name :    arduinoOS		 
            demo/int :         1234		 
         demo/double :      1234.12		 
         demo/string :         DEMO		..comment.. 
arduinoOS:/>|
```
| Command | Parameters | Description | Hidden |
| - | - | - | - |
| help | - | show aviable commands | yes |
| get | [parameter] | get a parameter (leave empty to list all parameter)| no |
| set | [parameter] [value] | set a parameter | no |
| load | - | load parameter from EEPROM (will automatically done on system start) | yes |
| save | - | save parameter to EEPROM (will automatically done on *get*) | yes |
| gpio | [gpio] [read/write] [0/1] | read or write GPIO | no |
| stats | - | show system information | no |
| clear | - | clear terminal | yes |
| reboot | - | reboot system | no |
| reset | - | reset EEPROM to default values and reboot | no |

* Reboot ist done by triggering WDT (default configured to 4 seconds)
* EEPROM will automatically reset after sketch upload

<br/><br/>
<img src="img/memory.png" width="25" align="right"/>
**Add a parameter**
```cpp
aos.addVariable("demo/int",  myInt, "My confg int", false, false);
```
* **Name**: Name of the parameter. Allowed data types: *char[]*.
* **Variable**: Allowed data types: *int*.
* **Description**: Desciption of your Parameter. Allowed data types: *char[]*.
* **Hidden**: Hide parameter in list. Allowed data types: *bool*.
* **Protected**: Don't allow to change the parameter. Allowed data types: *bool*.

<br/><br/>
**Add a command**
<img src="img/commands.png" width="25" align="right"/>
```cpp
aos.addCommand("demo",myFunction,"demo [parm1] [parm2] - My Function",false);
```
* **Name**: Name of the function. Allowed data types: *char[]*.
* **Function**: Allowed data types: void(*)(char**,uint8_t).
* **Description**: Desciption of your Function. Allowed data types: *char[]*.
* **Hidden**: Hide function in list. Allowed data types: *bool*.

<br/><br/>
**Begin**
```cpp
aos.begin(Serial);
```
* **Serial**: Reference to serial interface to start the Terminal. Allowed data types: *HardwareSerial*.
* NOTICE: You have to add all your parameter before calling *aos.begin(Serial)* to make the EEPROM layout clear to the system. 
* NOTICE: Your command-function have to take these parameters: void(char** param,uint8_t parCnt){};

<br/><br/>
**Example sketch:**
```cpp
#include <arduinoOS.h>

int     demoInt{1234};
double  demoDouble{1234.1234};
String  demoString{"DEMO"};

void demo(char** param,uint8_t parCnt){
    Serial.println("Hello from Demo Function");
    Serial.println("Parameter count: " + (String)parCnt);
    for(int i{0};i<parCnt;i++){
        Serial.println("- " + (String)param[i]);
    }
}

void setup()
{
    aos.addVariable("demo/int",     demoInt);
    aos.addVariable("demo/double",  demoDouble);
    aos.addVariable("demo/string",  demoString, "..comment..", false, false);
    aos.addCommand("demo",demo,"demo [parm1] [parm2] - ..comment..",false);
    aos.begin(Serial);
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