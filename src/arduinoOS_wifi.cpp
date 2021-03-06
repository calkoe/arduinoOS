#if defined ESP8266 || defined ESP32

#include "arduinoOS_wifi.h"

//Global
bool        ArduinoOS_wifi::telnet_enable{true};
bool        ArduinoOS_wifi::ntp_enable{false};
String      ArduinoOS_wifi::ntp_server{};
s16         ArduinoOS_wifi::ntp_offset{0};
bool        ArduinoOS_wifi::sta_enable{false};
String      ArduinoOS_wifi::sta_network{};
String      ArduinoOS_wifi::sta_password{};
String      ArduinoOS_wifi::sta_ip{"0.0.0.0"};
String      ArduinoOS_wifi::sta_subnet{"0.0.0.0"};
String      ArduinoOS_wifi::sta_gateway{"0.0.0.0"};
String      ArduinoOS_wifi::sta_dns{"0.0.0.0"};
bool        ArduinoOS_wifi::ap_enable{false};
String      ArduinoOS_wifi::ap_network{};
String      ArduinoOS_wifi::ap_password{};
WiFiUDP*    ArduinoOS_wifi::timeClientUDP{nullptr};
NTPClient*  ArduinoOS_wifi::timeClient{nullptr};
WiFiServer* ArduinoOS_wifi::TelnetServer{nullptr};
WiFiClient* ArduinoOS_wifi::TelnetClient{nullptr};
#if defined ESP32 
      SemaphoreHandle_t ArduinoOS_wifi::xBinarySemaphore;
#endif

void ArduinoOS_wifi::begin(){
    ArduinoOS::begin();
    wifi_config(0);
    eventListen("o",telnetOut);
    // DAEMON 5ms
    #if defined ESP8266
        setInterval([](){
            wifi_daemon();
        },5,"wifi_daemon");
    #endif
    #if defined ESP32 
        xTaskCreatePinnedToCore([](void* arg){
            while(true){
                xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
                ArduinoOS_wifi::wifi_daemon();
                xSemaphoreGive( xBinarySemaphore );
                vTaskDelay(5);
            }
        }, "wifi_daemon", 4096, NULL, 2, NULL, 1);
    #endif
};

void ArduinoOS_wifi::loop(){
    ArduinoOS::loop();
};

void ArduinoOS_wifi::wifi_daemon(){
    //Status
    if((ArduinoOS::status == 0 || ArduinoOS::status == 1) && connected()){
        ArduinoOS::status = 5;
    } 
    if(!connected()){
        ArduinoOS::status = 1;
    } 
    if(ap_enable) ArduinoOS::status = 0;
    if(!ap_enable && !sta_enable) ArduinoOS::status = 5;

    //NTP
    if(timeClient && connected()){
        timeClient->update();
    };

    //Telnet
    if(TelnetServer && connected()){
        // Cleanup disconnected session
        for(u8 i{0}; i < MAX_TELNET_CLIENTS; i++)
            if (TelnetClient[i] && !TelnetClient[i].connected())
                TelnetClient[i].stop();
        // Check new client connections
        if (TelnetServer->hasClient()){
            for(u8 i{0}; i < MAX_TELNET_CLIENTS; i++){
            if (!TelnetClient[i]){
                TelnetClient[i] = TelnetServer->available(); 
                TelnetClient[i].flush();
                o(0x07,false);
                p(textEscClear);
                p(textWelcome);
                commandList();           
                terminalPrefix();
                clearBuffer();
                break;
            };
            };
        };
        //Get Message
        for(u8 i{0}; i < MAX_TELNET_CLIENTS; i++)
            if (TelnetClient[i] && TelnetClient[i].connected())
                while(TelnetClient[i].available())
                    charIn(TelnetClient[i].read(),false);
    }  

    //Button
    if(ArduinoOS::bootButton>=0){
        static bool state = false;
        if(state != digitalRead(ArduinoOS::bootButton)){
            state = digitalRead(ArduinoOS::bootButton);
            if(!state){
                ap_enable = !ap_enable;
                variableLoad(true);
                wifi_config(1);
            }
        }
    }
};

//Methods
bool ArduinoOS_wifi::wifi_config(u8 s){
    if(s == 0){

        WiFi.setAutoConnect(true);
        WiFi.persistent(false); 
        WiFi.mode(WIFI_STA);

        #if defined ESP8266
            ESP.eraseConfig();
            WiFi.hostname(hostname); 
            WiFi.setSleepMode(WIFI_NONE_SLEEP,0);
            WiFi.setOutputPower(20.5);
        #endif

        #if defined ESP32
            WiFi.setHostname((const char*)hostname.c_str()); 
            WiFi.setSleep(true);
            WiFi.setTxPower(WIFI_POWER_19_5dBm);
        #endif


    };

    WiFi.mode(WIFI_OFF);

    //Telnet
    if(TelnetServer){
        TelnetServer->stop();
        delete TelnetServer;
        TelnetServer = nullptr;
    } 
    if(TelnetClient){
        for(u8 i{0}; i < MAX_TELNET_CLIENTS; i++)  
            TelnetClient[i].stop();  
        delete[] TelnetClient;    
        TelnetClient = nullptr;  
    } 
    if(telnet_enable){
        TelnetServer = new WiFiServer(23);
        TelnetClient = new WiFiClient[MAX_TELNET_CLIENTS];
        for(u8 i{0}; i < MAX_TELNET_CLIENTS; i++)
            TelnetClient[i].setTimeout(5000);       
        TelnetServer->begin();
        TelnetServer->setNoDelay(true);
    }

    //NTP
    if(timeClientUDP){
        timeClientUDP->stop();
        delete timeClientUDP;
        timeClientUDP = nullptr;
    }   
    if(timeClient){
        timeClient->end();
        delete timeClient;
        timeClient = nullptr;
    }     
    if(ntp_enable && ntp_server){
        timeClientUDP = new WiFiUDP;
        timeClientUDP->setTimeout(5000);
        timeClient = new NTPClient(*timeClientUDP);
        timeClient->setPoolServerName(ntp_server.c_str());
        timeClient->setTimeOffset(60 * 60 * ntp_offset);
        timeClient->setUpdateInterval(1000 * 60 * 10);  // 10 Min
        timeClient->begin();
    }

    //STA
    if(sta_enable && sta_network){
        if(sta_ip && sta_subnet && sta_gateway && sta_dns){
            IPAddress wifiIp;
            IPAddress wifiDns;
            IPAddress wifiGateway;
            IPAddress wifiSubnet; 
            wifiIp.fromString(sta_ip);
            wifiDns.fromString(sta_dns);
            wifiSubnet.fromString(sta_subnet);
            wifiGateway.fromString(sta_gateway);
            WiFi.config(wifiIp,wifiGateway,wifiSubnet,wifiDns);
        }
        WiFi.begin((const char*)sta_network.c_str(),(const char*)sta_password.c_str());
        //conf.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
        //conf.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    };

    //AP
    if(ap_enable && ap_network && !WiFi.softAPgetStationNum()){
        WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255, 255, 255, 0));
        WiFi.softAP((const char*)hostname.c_str(),(const char*)ap_password.c_str());
    };

    return true;
};
bool ArduinoOS_wifi::connected(){
    return (WiFi.status() == WL_CONNECTED && WiFi.localIP().toString() != "(IP unset)" && WiFi.localIP().toString() != "0.0.0.0");
}
inline s16 ArduinoOS_wifi::calcRSSI(s32 r){
    return min(max(2 * (r + 100.0), 0.0), 100.0);
};

void ArduinoOS_wifi::telnetOut(void* value){
    if(TelnetServer && telnet_enable){
        for(u8 i{0}; i < MAX_TELNET_CLIENTS; i++)
            if (TelnetClient[i] || TelnetClient[i].connected())
                TelnetClient[i].print((char*)value);
    }
};

//Interface Methods
ArduinoOS_wifi::ArduinoOS_wifi():ArduinoOS(){

    #if defined ESP32 
        xBinarySemaphore = xSemaphoreCreateBinary();
        xSemaphoreGive( xBinarySemaphore );
    #endif

    variableAdd("wifi/enable",       sta_enable,        "📶 Enable WiFi STA-Mode");
    variableAdd("wifi/network",      sta_network,       "📶 Network SSID");
    variableAdd("wifi/password",     sta_password,      "📶 Network Password");
    variableAdd("wifi/ip",           sta_ip,            "📶 IP (leave blank to use DHCP)");
    variableAdd("wifi/subnet",       sta_subnet,        "📶 Subnet");
    variableAdd("wifi/gateway",      sta_gateway,       "📶 Gateway");
    variableAdd("wifi/dns",          sta_dns,           "📶 DNS");
    variableAdd("hotspot/enable",    ap_enable,         "📶 Enable WiFi Hotspot-Mode");
    variableAdd("hotspot/password",  ap_password,       "📶 Hotspot Password");
    variableAdd("telnet/enable",     telnet_enable,     "📶 Enable Telnet support on Port 23 (require reboot)");
    variableAdd("ntp/enable",        ntp_enable,        "⏱  Enable NTP Sync");
    variableAdd("ntp/server",        ntp_server,        "⏱  NTP Server adress");
    variableAdd("ntp/offset",        ntp_offset,        "⏱  NTP Time offset");

    commandAdd("wifiStatus",[](char** param,u8 parCnt){
        o("");o("📶 Newtwork:");
        const char* s;
        switch(WiFi.status()){
            case WL_CONNECTED:      s = "WL_CONNECTED";break;
            case WL_NO_SHIELD:      s = "WL_NO_SHIELD";break;
            case WL_IDLE_STATUS:    s = "WL_IDLE_STATUS";break;
            case WL_CONNECT_FAILED: s = "WL_CONNECT_FAILED";break;
            case WL_NO_SSID_AVAIL:  s = "WL_NO_SSID_AVAIL";break;
            case WL_SCAN_COMPLETED: s = "WL_SCAN_COMPLETED";break;
            case WL_CONNECTION_LOST:s = "WL_CONNECTION_LOST";break;
            case WL_DISCONNECTED:   s = "WL_DISCONNECTED";break;
            default: s = "UNKOWN";
        };
        const char* m;
        switch(WiFi.getMode()){
            case WIFI_AP:           m = "WIFI_AP";break;
            case WIFI_STA:          m = "WIFI_STA";break;
            case WIFI_AP_STA:       m = "WIFI_AP_STA";break;
            case WIFI_OFF:          m = "WIFI_OFF";break;
            default:                m = "UNKOWN";
        };
        IPAddress localIP       = WiFi.localIP();
        IPAddress subnetMask    = WiFi.subnetMask();
        IPAddress gatewayIP     = WiFi.gatewayIP();
        IPAddress apIP          = WiFi.softAPIP();
        snprintf(OUT,LONG,"%-20s : %s","Mode",m);o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","Status",s);o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","Connected",connected()?"true":"false");o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","Hostname",hostname.c_str());o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","LocalMAC",WiFi.macAddress().c_str());o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","BSSID",WiFi.BSSIDstr().c_str());o(OUT);
        snprintf(OUT,LONG,"%-20s : %d.%d.%d.%d","LocalIP",localIP[0],localIP[1],localIP[2],localIP[3]);o(OUT);
        snprintf(OUT,LONG,"%-20s : %d.%d.%d.%d","SubnetMask",subnetMask[0],subnetMask[1],subnetMask[2],subnetMask[3]);o(OUT);
        snprintf(OUT,LONG,"%-20s : %d.%d.%d.%d","GatewayIP",gatewayIP[0],gatewayIP[1],gatewayIP[2],gatewayIP[3]);o(OUT);
        snprintf(OUT,LONG,"%-20s : %d dBm (%d%%)","RSSI",WiFi.RSSI(),calcRSSI(WiFi.RSSI()));o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","AP MAC",WiFi.softAPmacAddress().c_str());o(OUT);
        snprintf(OUT,LONG,"%-20s : %d.%d.%d.%d","AP IP",apIP[0],apIP[1],apIP[2],apIP[3]);o(OUT);
        snprintf(OUT,LONG,"%-20s : %d","AP Stations",WiFi.softAPgetStationNum());o(OUT);
        if(timeClient) snprintf(OUT,LONG,"%-20s : %s","NTP Time",timeClient->getFormattedTime().c_str());o(OUT);
    },  "📶 Shows System / Wifi status");
    
    #if defined ESP8266
        commandAdd("wifiFirmware",[](char** param,u8 parCnt){
            if(parCnt==2){
                String url = param[1];
                snprintf(OUT,LONG,"[HTTP UPDATE] Requesting: %s",url.c_str());o(OUT);
                ESPhttpUpdate.setLedPin(STATUSLED,0);
                ESPhttpUpdate.onProgress([](u32 p,u32 t){
                    snprintf(OUT,LONG,"[HTTP UPDATE] Downloading: %u of %u Bytes",p,t);o(OUT);
                });
                t_httpUpdate_return ret;
                if(url.startsWith("https")){
                    WiFiClientSecure net;
                        net.setInsecure();
                    ret = ESPhttpUpdate.update(net, param[1], firmware);
                }else{
                    WiFiClient net;
                    ret = ESPhttpUpdate.update(net, param[1], firmware);
                } 
                switch(ret) {
                    case HTTP_UPDATE_FAILED:
                        snprintf(OUT,LONG,"[HTTP UPDATE] Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());o(OUT);
                        break;
                    case HTTP_UPDATE_NO_UPDATES:
                        o("[HTTP UPDATE] No Update Aviable.");
                        break;
                    case HTTP_UPDATE_OK:
                        o("[HTTP UPDATE] Update ok.");
                        break;
                }
            }else{
                commandMan("firmware");
            }
        },"📶 [url] | load and install new firmware from URL (http or https)");
     #endif
    
    #if defined ESP32
        commandAdd("wifiFirmware",[](char** param,u8 parCnt){
            if(parCnt==2){
                String url = param[1];
                snprintf(OUT,LONG,"[HTTP UPDATE] Requesting: %s",url.c_str());o(OUT);
                httpUpdate.setLedPin(STATUSLED,0);
                HTTPUpdateResult ret;
                if(url.startsWith("https")){
                    WiFiClientSecure net;
                    ret = httpUpdate.update(net, param[1], firmware);
                }else{
                    WiFiClient net;
                    ret = httpUpdate.update(net, param[1], firmware);
                } 
                switch(ret) {
                    case HTTP_UPDATE_FAILED:
                        snprintf(OUT,LONG,"[HTTP UPDATE] Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());o(OUT);
                        break;
                    case HTTP_UPDATE_NO_UPDATES:
                        o("[HTTP UPDATE] No Update Aviable.");
                        break;
                    case HTTP_UPDATE_OK:
                        o("[HTTP UPDATE] Update ok.");
                        break;
                }
            }else{
                commandMan("firmware");
            }
        },"📶 [url] | load and install new firmware from URL (http or https)");
     #endif

    commandAdd("wifiScan",[](char** param,u8 parCnt){
        o("Scaning for Networks...");
        u8 n = WiFi.scanNetworks();
        if(n){
            for (u8 i = 0; i < n; i++){
                const char* e;
                switch(WiFi.encryptionType(i)){
                    case 0: e = "OPEN";break;
                    case 1: e = "WEP";break;
                    case 2: e = "WPA_PSK";break;
                    case 3: e = "WPA2_PSK";break;
                    case 4: e = "WPA_WPA2_PSK";break;
                    case 5: e = "WPA2_ENTERPRISE";break;
                    case 6: e = "AUTH_MAX";break;
                    default:e = "UNKOWN";
                }
                snprintf(OUT,LONG,"%-20s : %d dBm (%d%%) (%s) BSSID: %s",WiFi.SSID(i).c_str(),WiFi.RSSI(i), calcRSSI(WiFi.RSSI(i)),e,WiFi.BSSIDstr(i).c_str());o(OUT);
            }
        }else o("❌ No Networks found!");
    },    "📶 Scans for nearby networks");

    commandAdd("wifiConnect",[](char** param,u8 parCnt){
        #if defined ESP32 
            xSemaphoreTake( xBinarySemaphore, portMAX_DELAY );
        #endif

        if(parCnt>=2){
                snprintf(OUT,LONG,"Set  wifi/enabled: %s","true");o(OUT);
                sta_enable  = true;
                snprintf(OUT,LONG,"Set  wifi/network: %s",param[1]);o(OUT);
                variableSet("wifi/network",param[1]);
        };
        if(parCnt>=3){
                snprintf(OUT,LONG,"Set wifi/password: %s",param[2]);o(OUT);
                variableSet("wifi/password",param[2]);
        };
        variableLoad(true);
        o("DONE! ✅ > Type 'wifiStatus' to check status");
        wifi_config(1);

        #if defined ESP32 
            xSemaphoreGive( xBinarySemaphore );
        #endif

    }, "📶 [network] [password] | apply network settings and connect to configured network",false);

    commandAdd("wifiDns",[](char** param,u8 parCnt){
        if(parCnt==2){
            IPAddress remote_addr;
            if(WiFi.hostByName(param[1], remote_addr)){
                snprintf(OUT,LONG,"✅ DNS %s -> %d.%d.%d.%d",param[1],remote_addr[0],remote_addr[1],remote_addr[2],remote_addr[3]);o(OUT);
            }else{
                snprintf(OUT,LONG,"❌ DNS lookup failed");o(OUT);
            };
        }},    "📶 [ip] | check internet connection");
};


#endif