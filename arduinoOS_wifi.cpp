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

};
void arduinoOS_wifi::loop(){
    aos.loop();
}

//Interface Methods
void arduinoOS_wifi::status(char**,uint8_t){
    aos.o("Hello from status Function");
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
