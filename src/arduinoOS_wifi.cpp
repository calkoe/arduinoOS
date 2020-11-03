#include <arduinoOS_wifi.h>
#ifdef ESP8266

//Global
bool        ArduinoOS_wifi::telnet_enable{true};
bool        ArduinoOS_wifi::ntp_enable{false};
String      ArduinoOS_wifi::ntp_server{};
s32         ArduinoOS_wifi::ntp_offset{0};
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
WiFiUDP     ArduinoOS_wifi::wifiUDP;
NTPClient   ArduinoOS_wifi::timeClient{ArduinoOS_wifi::wifiUDP};
WiFiServer* ArduinoOS_wifi::TelnetServer;
WiFiClient* ArduinoOS_wifi::TelnetClient;

void ArduinoOS_wifi::begin(){
    ArduinoOS::begin();
    config(0);
    //1S set Status
    setInterval([](){
        if(sta_enable && sta_network && sta_connected()){
            if(ArduinoOS::status == 1) ArduinoOS::status = 5;
        }
        else{
            if(ArduinoOS::status == 5 && sta_enable) ArduinoOS::status = 1;
        } 
    },1000);
    //10S Time update
    setInterval([](){
        if(ntp_enable && ntp_server && sta_connected()){
            timeClient.update();
        };
    },10000);
    //LOOP 10ms
    setInterval([](){
        if(telnet_enable) telnetLoop();
    },10);
};
void ArduinoOS_wifi::loop(){
    ArduinoOS::loop();
    yield();
    optimistic_yield(100);
};

//Methods
bool ArduinoOS_wifi::config(u8 s){
    if(s == 0){
        //WiFi
        ESP.eraseConfig();
        WiFi.setAutoConnect(true);
        WiFi.persistent(false); 
        WiFi.mode(WIFI_STA);
        WiFi.hostname(hostname); 
        WiFi.mode(WIFI_OFF);
        //wifi_set_sleep_type(NONE_SLEEP_T); //https://blog.creations.de/?p=149 //Remove ??

        //Telnet
        if(TelnetServer) delete TelnetServer;
        if(TelnetClient) delete[] TelnetClient;
        if(telnet_enable){
            TelnetServer = new WiFiServer(23);
            TelnetClient = new WiFiClient[MAX_TELNET_CLIENTS];
            eventListen("o",telnetOut);
            TelnetServer->begin();
            TelnetServer->setNoDelay(true);
        }
        
    };

    //NTP
    if(ntp_enable && ntp_server){
        timeClient.setPoolServerName(ntp_server.c_str());
        timeClient.setTimeOffset(60 * 60 * ntp_offset);
        timeClient.setUpdateInterval(1000 * 60 * 60);
        timeClient.begin();
    }

    if(sta_enable && sta_network && sta_connected){
        if(sta_ip && sta_subnet && sta_gateway && sta_dns){
            IPAddress wifiIp;
            IPAddress wifiDns;
            IPAddress wifiGateway;
            IPAddress wifiSubnet; 
            wifiIp.fromString(sta_ip);
            wifiDns.fromString(sta_dns);
            wifiSubnet.fromString(sta_subnet);
            wifiGateway.fromString(sta_gateway);
            WiFi.config(wifiIp,wifiDns,wifiSubnet,wifiGateway);
        }
        WiFi.begin(sta_network,sta_password);
    };
    if(ap_enable && ap_network && !WiFi.softAPgetStationNum()){
        WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255, 255, 255, 0));
        WiFi.softAP(hostname,ap_password);
    };
    if(!sta_enable && !ap_enable) WiFi.mode(WIFI_OFF);  //Remove ??

    return true;
};
bool ArduinoOS_wifi::sta_connected(){
    return (WiFi.status() == WL_CONNECTED && WiFi.localIP().toString() != "(IP unset)" && WiFi.localIP().toString() != "0.0.0.0");
}
inline s32 ArduinoOS_wifi::calcRSSI(s32 r){
    return min(max(2 * (r + 100.0), 0.0), 100.0);
};

//Telnet
void ArduinoOS_wifi::telnetLoop(){
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
        commandList();             //Unfify with Serial ??
        terminalPrefix();
        clearBuffer(IO[IOC],LONG);
        break;
      };
    };
  };
  //Get Message
  for(u8 i{0}; i < MAX_TELNET_CLIENTS; i++)
    if (TelnetClient[i] && TelnetClient[i].connected())
        while(TelnetClient[i].available()){
            while(TelnetClient[i].available())
                charIn(TelnetClient[i].read(),false);
            delay(1);
        };
};
void ArduinoOS_wifi::telnetOut(void* value){
    for(u8 i{0}; i < MAX_TELNET_CLIENTS; i++)
        if (TelnetClient[i] || TelnetClient[i].connected())
            TelnetClient[i].print((char*)value);
};

//Interface Methods
ArduinoOS_wifi::ArduinoOS_wifi():ArduinoOS(){
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
        char* s;
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
        char* m;
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
        snprintf(OUT,LONG,"%-20s : %s","Connected",sta_connected()?"true":"false");o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","Hostname",hostname.c_str());o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","LocalMAC",WiFi.macAddress().c_str());o(OUT);
        snprintf(OUT,LONG,"%-20s : %d.%d.%d.%d","LocalIP",localIP[0],localIP[1],localIP[2],localIP[3]);o(OUT);
        snprintf(OUT,LONG,"%-20s : %d.%d.%d.%d","SubnetMask",subnetMask[0],subnetMask[1],subnetMask[2],subnetMask[3]);o(OUT);
        snprintf(OUT,LONG,"%-20s : %d.%d.%d.%d","GatewayIP",gatewayIP[0],gatewayIP[1],gatewayIP[2],gatewayIP[3]);o(OUT);
        snprintf(OUT,LONG,"%-20s : %d dBm (%d%%)","RSSI",WiFi.RSSI(),calcRSSI(WiFi.RSSI()));o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","AP MAC",WiFi.softAPmacAddress().c_str());o(OUT);
        snprintf(OUT,LONG,"%-20s : %d.%d.%d.%d","AP IP",apIP[0],apIP[1],apIP[2],apIP[3]);o(OUT);
        snprintf(OUT,LONG,"%-20s : %d","AP Stations",WiFi.softAPgetStationNum());o(OUT);
        snprintf(OUT,LONG,"%-20s : %s","NTP Time",timeClient.getFormattedTime().c_str());o(OUT);
    },  "📶 Shows System / Wifi status");

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

    commandAdd("wifiScan",[](char** param,u8 parCnt){
        o("Scaning for Networks...");
        u8 n = WiFi.scanNetworks();
        if(n){
            for (u8 i = 0; i < n; i++){
                char* e;
                switch(WiFi.encryptionType(i)){
                    case AUTH_OPEN:         e = "AUTH_OPEN";break;
                    case AUTH_WEP:          e = "AUTH_WEP";break;
                    case AUTH_WPA_PSK:      e = "AUTH_WPA_PSK";break;
                    case AUTH_WPA2_PSK:     e = "AUTH_WPA2_PSK";break;
                    case AUTH_WPA_WPA2_PSK: e = "AUTH_WPA_WPA2_PSK";break;
                    default:                e = "UNKOWN";
                }
                snprintf(OUT,LONG,"%-20s : %d dBm (%d%%) (%s)",WiFi.SSID(i).c_str(),WiFi.RSSI(i), calcRSSI(WiFi.RSSI(i)),e);o(OUT);
            }
        }else o("❌ No Networks found!");
    },    "📶 Scans for nearby networks");

    commandAdd("wifiConnect",[](char** param,u8 parCnt){
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
        config(1);
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