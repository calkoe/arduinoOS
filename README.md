<img src="img/arduinoOS.png" width="300"></img>

**A simple library for saving and modify your applications setting (ESP8266 with WiFi and MQTT Support)**

**Features:**
* UNIX style serial Terminal
* EEProm support for saving parameters
* Enables access to application-parameters
* Enables access to application-functions
* Int, Double and String datatypes supported for parameters
* Robust resign with no Heap-Fragmentation
* Small Footprint (1k of SRAM)

**Serial Terminal:**
```
ArduinOS V1.0
Commands:
                demo : demo [parm1] [parm2] - Runs the Demo Com
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
         demo/string :         DEMO		This is a Demo String! 
arduinoOS:/>
```

**...to be continued...**
<!--<img src="img/memory.png" width="25" style="float:right"/>
<img src="img/commands.png" width="25" style="float:right"/>
<img src="img/plugins.png" width="25" style="float:right"/>-->
