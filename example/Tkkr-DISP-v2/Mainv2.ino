#include <arduino.h>

#include <display-cmd.h>


#include "pin_config.h"


void panic(const String& message) {
    display_big(message, TFT_RED);
    delay(5000);
    ESP.restart();
}

// ******************* SetUp ************************************** SetUp ************************************* SetUp *******************

void setup() {
    
    Serial.begin(115200);
    delay(2000);

    Serial.println("MQTTGo DISP v2 start");

    display_init();
    Serial.println("Init done, now logo");
   
    display_logo();
    delay(1000);


    Serial.println("Setup done, now loop....");
}


// ******************* Loop ************************************** Loop ************************************** Loop *******************

#define every(t) for (static unsigned long _lasttime; (unsigned long)((unsigned long)millis() - _lasttime) >= (t); _lasttime = millis())


String          mqtt_Pubtopic;
String          mqtt_Subtopic;


void MQTTpub(const String& topic, const String& message) {
    Serial.printf("MQTTGo v2 pub-msg: topic: %s payload: %s\n", topic.c_str(), message.c_str());
    display_topic(topic, message);
}

void MQTTsub(const String& topic, const String& message) {
    Serial.printf("MQTTGo v2 Pub-ing ret. message now... topic: %s payload: %s\n", topic.c_str(), message.c_str());
    Serial.printf("MQTTGo v2 Incomming msg: topic: %s payload: %s\n", topic.c_str(), message.c_str());
    display_Incoming_topic(topic, message);
}


void MQTTinfo (const String& broker, const String& mqtt_topic, const String& mqtt_sub_topic)
{
    display_config(broker, mqtt_topic, mqtt_sub_topic);
    
}

void loop() {
    
    Serial.println("Loop ..");
    

    delay (2000);

/*  void display_init();
    void display_big(const String& text, int fg = TFT_WHITE, int bg = TFT_BLACK);
    void display_lines(const std::list<String>& lines, int fg = TFT_WHITE, int bg = TFT_BLACK);
    void display_logo();
    void display_topic(const String& mqtt_topic, const String& payload);
    void display_Incoming_topic(const String& mqtt_in_topic, const String& payload);
    void display_config(const String& broker, const String& mqtt_topic, const String& mqtt_sub_topic);
*/
    display_big("Disp Big");
    delay (2000);


    MQTTpub( "topic-out",  "message OUT");
    delay(2000);

    MQTTsub( "topic-in",  "message IN");
    delay(2000);

    MQTTinfo("Broker","PUB topic", "SUB-Topic");
    delay(2000);

}   //loop
