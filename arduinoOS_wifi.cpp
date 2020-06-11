#include "arduinoOS_wifi.h"
arduinoOS_wifi aos_wifi;

//Global
arduinoOS_wifi::arduinoOS_wifi(){};
void arduinoOS_wifi::begin(){

    //Settings
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

    //Interface
    aos.addCommand("wifi-status",status,"Shows the current connetion status",false);
    aos.addCommand("wifi-scan",sta_scan,"Scans for nearby networks",false);
    aos.addCommand("wifi-connect",sta_connect,"connect to configured a network",false);
    aos.addCommand("wifi-ping",sta_ping,"ping [ip] - check internet connection",false);

    aos.begin(Serial);

    //Connect to WiFi

};
void arduinoOS_wifi::loop(){
    aos.loop();
}

//Interface Methods
void arduinoOS_wifi::status(char**,uint8_t){
    aos.o("WIFI Status:");
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
    unsigned  RssiQual      = min(max(2 * (WiFi.RSSI() + 100.0), 0.0), 100.0);
    IPAddress localIP       = WiFi.localIP();
    IPAddress subnetMask    = WiFi.subnetMask();
    IPAddress gatewayIP     = WiFi.gatewayIP();
    snprintf(aos.charIOBuffer,LONG,"%-20s : %s","Mode",m);aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %s","Status",s);aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %d.%d.%d.%d","LocalIP",localIP[0],localIP[1],localIP[2],localIP[3]);aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %d.%d.%d.%d","SubnetMask",subnetMask[0],subnetMask[1],subnetMask[2],subnetMask[3]);aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %d.%d.%d.%d","GatewayIP",gatewayIP[0],gatewayIP[1],gatewayIP[2],gatewayIP[3]);aos.o(aos.charIOBuffer);
    snprintf(aos.charIOBuffer,LONG,"%-20s : %d dBm (%d%%)","RSSI",WiFi.RSSI(),RssiQual);aos.o(aos.charIOBuffer);
};
void arduinoOS_wifi::sta_scan(char**,uint8_t){
    aos.o("Hello from sta_scan Function");
};
void arduinoOS_wifi::sta_connect(char**,uint8_t){
    aos.o("Hello from sta_connect Function");
};
void arduinoOS_wifi::sta_ping(char**,uint8_t){
    aos.o("Hello from sta_ping Function");
};
