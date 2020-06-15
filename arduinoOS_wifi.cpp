#include "arduinoOS_wifi.h"
arduinoOS_wifi aos_wifi;

//Global
arduinoOS_wifi::arduinoOS_wifi(){
    aos.addVariable("wifi/enabled",      sta_enabled);
    aos.addVariable("wifi/network",      sta_network);
    aos.addVariable("wifi/password",     sta_password);
    aos.addVariable("wifi/ip",           sta_ip, "", true, false);
    aos.addVariable("wifi/subnet",       sta_subnet, "",true, false);
    aos.addVariable("wifi/gateway",      sta_gateway, "", true, false);
    aos.addVariable("wifi/dns",          sta_dns, "", true, false);
    aos.addVariable("hotspot/ap_enabled",ap_enabled);
    aos.addVariable("hotspot/network",   ap_network);
    aos.addVariable("hotspot/password",  ap_password);
    aos.addCommand("network",wifi_status,"📶 Shows the current connetion status",false);
    aos.addCommand("scan",sta_scan,"📶 Scans for nearby networks",false);
    aos.addCommand("connect",sta_connect,"📶 connect to configured a network",false);
    aos.addCommand("dns",sta_ping,"📶 dns [ip] - check internet connection",false);
};
void arduinoOS_wifi::begin(){
    wifi_config(0);
};
void arduinoOS_wifi::loop(){
    aos.loop();
}

//Interface Methods
void arduinoOS_wifi::wifi_status(char**,uint8_t){
    aos.o("Newtwork Status:");
    char* s;
    switch(WiFi.status()){
        case WL_CONNECTED:
            s = "WL_CONNECTED";break;
        case WL_NO_SHIELD:
            s = "WL_NO_SHIELD";break;
        case WL_IDLE_STATUS:
            s = "WL_IDLE_STATUS";break;
        case WL_CONNECT_FAILED:
            s = "WL_CONNECT_FAILED";break;
        case WL_NO_SSID_AVAIL:
            s = "WL_NO_SSID_AVAIL";break;
        case WL_SCAN_COMPLETED:
            s = "WL_SCAN_COMPLETED";break;
        case WL_CONNECTION_LOST:
            s = "WL_CONNECTION_LOST";break;
        case WL_DISCONNECTED:
            s = "WL_DISCONNECTED";break;
        default:
            s = "UNKOWN";
    };
    char* m;
    switch(WiFi.getMode()){
        case WIFI_AP:
            m = "WIFI_AP";break;
        case WIFI_STA:
            m = "WIFI_STA";break;
        case WIFI_AP_STA:
            m = "WIFI_AP_STA";break;
        case WIFI_OFF:
            m = "WIFI_OFF";break;
        default:
            char* m = "UNKOWN";
    };
    IPAddress localIP       = WiFi.localIP();
    IPAddress subnetMask    = WiFi.subnetMask();
    IPAddress gatewayIP     = WiFi.gatewayIP();
    IPAddress apIP          = WiFi.softAPIP();
    snprintf(aos.charIOBuffer,LONG,"%-20s : %s","Mode",m);aos.o(aos.charIOBuffer);
    aos.o("");
    snprintf(aos.charIOBuffer,LONG,"%-20s : %s","Status",s);aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %s","LocalMAC",WiFi.macAddress().c_str());aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %d.%d.%d.%d","LocalIP",localIP[0],localIP[1],localIP[2],localIP[3]);aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %d.%d.%d.%d","SubnetMask",subnetMask[0],subnetMask[1],subnetMask[2],subnetMask[3]);aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %d.%d.%d.%d","GatewayIP",gatewayIP[0],gatewayIP[1],gatewayIP[2],gatewayIP[3]);aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %d dBm (%d%%)","RSSI",WiFi.RSSI(),calcRSSI(WiFi.RSSI()));aos.o(aos.charIOBuffer);
    aos.o("");
    snprintf(aos.charIOBuffer,LONG,"%-20s : %s","AP MAC",WiFi.softAPmacAddress().c_str());aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %d.%d.%d.%d","AP IP",apIP[0],apIP[1],apIP[2],apIP[3]);aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %d","AP Stations",WiFi.softAPgetStationNum());aos.o(aos.charIOBuffer);
};
void arduinoOS_wifi::wifi_config(uint8_t s){
    //Source
    // 0 - Node
    // 1 - STA
    // 2 - AP
    
    if(s == 0){
        ESP.eraseConfig();
        WiFi.mode(WIFI_OFF);
        WiFi.setAutoConnect(true);
        WiFi.persistent(false); 
        //wifi_set_sleep_type(NONE_SLEEP_T); //https://blog.creations.de/?p=149
    };

    if(s == 0 || s == 1){
        if(aos_wifi.sta_enabled){
            WiFi.hostname(aos.aos_hostname); 
            if(aos_wifi.sta_ip && aos_wifi.sta_subnet && aos_wifi.sta_gateway && aos_wifi.sta_dns){
                IPAddress wifiIp;
                IPAddress wifiDns;
                IPAddress wifiGateway;
                IPAddress wifiSubnet; 
                WiFi.config (   wifiIp.fromString(aos_wifi.sta_ip),
                                wifiDns.fromString(aos_wifi.sta_dns),
                                wifiSubnet.fromString(aos_wifi.sta_subnet),
                                wifiGateway.fromString(aos_wifi.sta_gateway));
            }
            WiFi.begin(aos_wifi.sta_network,aos_wifi.sta_password);
        }else if(aos_wifi.ap_enabled)
            WiFi.mode(WIFI_AP);
        else
            WiFi.mode(WIFI_OFF);
    }

    if(s == 0 || s == 2){
        if(aos_wifi.ap_enabled){
            WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255, 255, 255, 248));
            WiFi.softAP(aos_wifi.ap_network,aos_wifi.ap_password);
        }else if(aos_wifi.sta_enabled)
            WiFi.mode(WIFI_STA);
        else
            WiFi.mode(WIFI_OFF);
    };
};
inline int arduinoOS_wifi::calcRSSI(int r){
    return min(max(2 * (r + 100.0), 0.0), 100.0);
};
void arduinoOS_wifi::sta_scan(char**,uint8_t){
    aos.o("Scaning for Networks...");
    uint8_t n = WiFi.scanNetworks();
    if(n){
        for (uint8_t i = 0; i < n; i++){
            char* e;
            switch(WiFi.encryptionType(i)){
                case AUTH_OPEN:
                    e = "AUTH_OPEN";break;
                case AUTH_WEP:
                    e = "AUTH_WEP";break;
                case AUTH_WPA_PSK:
                    e = "AUTH_WPA_PSK";break;
                case AUTH_WPA2_PSK:
                    e = "AUTH_WPA2_PSK";break;
                case AUTH_WPA_WPA2_PSK:
                    e = "AUTH_WPA_WPA2_PSK";break;
                default:
                    char* m = "UNKOWN";
            }
            snprintf(aos.charIOBuffer,LONG,"%-20s : %d dBm (%d%%) (%s)",WiFi.SSID(i).c_str(),WiFi.RSSI(i), calcRSSI(WiFi.RSSI()),e);aos.o(aos.charIOBuffer);
        }
    }else aos.o("❌ No Networks found!");
};

void arduinoOS_wifi::sta_connect(char** c,uint8_t n){
    if(n==3){
            snprintf(aos.charIOBuffer,LONG,"Set  wifi/enabled: %s","true");aos.o(aos.charIOBuffer);
            snprintf(aos.charIOBuffer,LONG,"Set  wifi/network: %s",c[1]);aos.o(aos.charIOBuffer);
            snprintf(aos.charIOBuffer,LONG,"Set wifi/password: %s",c[2]);aos.o(aos.charIOBuffer);
            aos_wifi.sta_enabled  = true;
            aos_wifi.sta_network  = c[1];
            aos_wifi.sta_password = c[2];
            aos.loadVariables(true);
    };
    aos.o("Connecting");
    aos.o("Type 'network' to check status");
    wifi_config(1);
};
void arduinoOS_wifi::sta_ping(char** c,uint8_t n){
    if(n==2){
        IPAddress remote_addr;
        if(WiFi.hostByName(c[1], remote_addr)){
            snprintf(aos.charIOBuffer,LONG,"✅ DNS %s -> %d.%d.%d.%d",c[1],remote_addr[0],remote_addr[1],remote_addr[2],remote_addr[3]);aos.o(aos.charIOBuffer);
        }else{
            snprintf(aos.charIOBuffer,LONG,"❌ DNS lookup failed");aos.o(aos.charIOBuffer);
        };
        /*for(uint8_t i{0};i<1;i++){
            PingClass Ping;
            if(Ping.ping(remote_addr,1)){
                snprintf(aos.charIOBuffer,LONG,"[%d] Success, RTT: %dms",i,Ping.maxTime());aos.o(aos.charIOBuffer);
            }else{
                snprintf(aos.charIOBuffer,LONG,"[%d] Ping error",i);aos.o(aos.charIOBuffer);
            }
        };*/
    };
};