/**
 * 
 * Dependencies:
 *      https://github.com/calkoe/arduinoOS
 *      https://github.com/bblanchon/ArduinoJson
 *      https://github.com/adafruit/Adafruit_BusIO
 *      https://github.com/adafruit/Adafruit_SSD1306
 */

#include <arduinoOS_mqtt.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

ArduinoOS_mqtt aos;
Adafruit_SSD1306 display(128, 32, &Wire);
StaticJsonDocument<200> doc;

void setup()
{
    aos.autoReset=false;
    aos.begin();

    //Setup Display
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.clearDisplay();
    display.setTextSize(4);             
    display.setTextColor(WHITE); 

    aos.subscripe("my/own/datatopic",0,[](char* t, char* payload){
        deserializeJson(doc, payload);
        display.clearDisplay();
        display.setCursor(0,0); 
        display.println((float)(doc["kmh"]));
        display.display();
    });

}

void loop()
{ 
    aos.loop();
}
