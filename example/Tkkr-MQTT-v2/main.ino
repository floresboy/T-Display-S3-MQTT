/*
    WifiSettings advanced example

    Demonstrates callback functions and custom variables
    to be saved through WifiSettings.

    Source and further documentation available at
    https://github.com/Juerd/ESP-WiFiSettings

    Note: this example is written for ESP32.
    For ESP8266, use LittleFS.begin() instead of SPIFFS.begin(true).


    https://github.com/Juerd/snuffelding/blob/main/snuffelding.ino
*/

#include <SPIFFS.h>
#include <WiFiSettings.h>
#include <MQTT.h>
#include "pin_config.h"
#include <MQTT_strings.h> 
#include <WiFi.h>
#include "display-cmd.h"

MQTTLanguage::Texts T;
MQTTClient      mqtt;



#define LED_ON  LOW
#define LED_OFF HIGH

// Global vars: configuration via WiFiSettings
unsigned long   mqtt_interval;
String          mqtt_Pubtopic;
String          mqtt_Subtopic;
String          mqtt_template;
bool            wifi_enabled;
bool            mqtt_enabled;
int             max_failures;
String          mqtt_broker;
int             mqtt_port;

#define LANGUAGE "en"

void panic(const String& message) {
    display_big(message, TFT_RED);
    delay(5000);
    ESP.restart();
}

// ****************** MQTT *******************

void connect_mqtt() {
    if (mqtt.connected()) return;  // already/still connected - https://github.com/256dpi/arduino-mqtt

    static int failures = 0;
    if (mqtt.connect(WiFiSettings.hostname.c_str())) {
        failures = 0;
        mqtt.subscribe(mqtt_Subtopic);
    } else {
        failures++;
        if (failures >= max_failures) panic(T.error_mqtt);
    }
}

void MQTTpub(const String& topic, const String& message) {
// Handler for to be published messages
    Serial.printf("publishing on topic: %s payload: %s\n", topic.c_str(), message.c_str());
    display_topic(topic, message);
    mqtt.publish(topic, message, false, 0);  //bool .publish(const char topic[], const char payload[], bool retained, int qos);
}

void MQTTretain(const String& topic, const String& message) {
    Serial.printf("MQTTGo v2 Pub-ing ret. message now... topic: %s payload: %s\n", topic.c_str(), message.c_str());
    display_topic(topic, message);
    mqtt.publish(topic, message, true, 0);
}

void MQTT_messageReceived(String &topic, String &payload) {
// callback function for incomming (subscribed messages)
  Serial.println("incoming on topic: " + topic + " payload: " + payload);
  display_Incoming_topic(topic, payload);
 }

// ########################################################################

void Report_Global_Settings(void)
{   Serial.println("*** WiFi Settings overview ***");
    Serial.print("mqtt_broker: ");
    Serial.println(mqtt_broker);
    Serial.print("mqtt_port: ");
    Serial.println(mqtt_port);
    Serial.print("mqtt_Pubtopic: ");
    Serial.println(mqtt_Pubtopic);
    Serial.print("mqtt_Subtopic: ");
    Serial.println(mqtt_Subtopic);
    Serial.print("mqtt_template/payload: ");
    Serial.println(mqtt_template);
    // Serial.print("PlatformIO Unix compile time: ");
    // Serial.println(COMPILE_UNIX_TIME);
    Serial.println("*******************");
}


void Setup_wifi_connection(void)
{
    // Set wifi connection

    // Define custom settings saved by WifiSettings portal
    // These will return the default if nothing was set before
    WiFiSettings.hostname = "HiveMQ-";
    WiFiSettings.language = LANGUAGE;
    WiFiSettings.heading("MQTT");
    MQTTLanguage::select(T, WiFiSettings.language);
    // Create unique SSID from hostname
    for (auto& str : T.portal_instructions[0]) {
        str.replace("{ssid}", WiFiSettings.hostname);
    }
    
    mqtt_enabled  = WiFiSettings.checkbox("HiveMQ_mqtt", true, T.config_mqtt) && wifi_enabled;
    mqtt_broker = WiFiSettings.string("mqtt_broker", 64, "broker.hivemq.com", T.config_mqtt_server);
    mqtt_port      = WiFiSettings.integer("mqtt_port", 0, 65535, 1883, T.config_mqtt_port);
    max_failures  = WiFiSettings.integer("HiveMQ_max_failures", 0, 1000, 10, T.config_max_failures);
    mqtt_Pubtopic = WiFiSettings.string("HiveMQ_mqtt_topic", "HiveMQ/"+WiFiSettings.hostname, T.config_mqtt_topic);
    mqtt_Subtopic  = WiFiSettings.string("HiveMQ_Submqtt_topic", "HiveMQ/SubTopic", T.config_Submqtt_topic); 
    mqtt_interval = 1000UL * WiFiSettings.integer("HiveMQ_mqtt_interval", 10, 3600, 30, T.config_mqtt_interval);
    mqtt_template = WiFiSettings.string("HiveMQ_mqtt_template", "Payload : {}", T.config_mqtt_template);
    WiFiSettings.info(T.config_template_info);
    
    // Set custom callback functions
    WiFiSettings.onSuccess  = []() {
        digitalWrite(PIN_LCD_BL, LED_ON); // Turn LED on
    };

    WiFiSettings.onFailure  = []() {
        digitalWrite(PIN_LCD_BL, LED_ON); // Turn LED off
        display_big(T.error_wifi, TFT_RED);
        delay(2000);
        WiFiSettings.portal();
        Serial.println("wifi settings Failure, Wifi config portal (re)started - ");
    };
    WiFiSettings.onWaitLoop = []() {
        digitalWrite(PIN_LCD_BL, !digitalRead(PIN_LCD_BL)); // Toggle LED
        return 500; // Delay next function call by 500ms
    };

    Serial.println("Attempting to connect with these parameters : ");
    Report_Global_Settings();
    // Connect to WiFi with a timeout of 30 seconds
    // Launches the portal if the connection failed
    WiFiSettings.connect(true, 30);
}


bool button(int pin) {
    if (digitalRead(pin)) return false;
    unsigned long start = millis();
    while (!digitalRead(pin)) {
        if (millis() - start >= 50) Serial.println("In button, returning true");
        return(true);
    }
    return millis() - start >= 50;
}


void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n\n************");
    Serial.println("In setup ..");
    Serial.println("************");
    SPIFFS.begin(true);  // Will format on the first run after failing to mount
    pinMode(PIN_LCD_BL, OUTPUT);
    pinMode(PIN_BUTTON_1, INPUT);
    pinMode(PIN_BUTTON_2, INPUT);

    display_init();
    Serial.println("Init done, now logo");
    display_logo();
    delay(5000);
    Serial.println("Logo done, now setup wifi");
    Setup_wifi_connection();
    Report_Global_Settings();
    Serial.println("wifi settings Success - Now initiating MQTT ");

    mqtt_enabled = true;
    static WiFiClient wificlient;
    if (mqtt_enabled) {
        mqtt.begin(mqtt_broker.c_str(), mqtt_port, wificlient);
        mqtt.onMessage(MQTT_messageReceived);
    }

    MQTTretain(mqtt_Pubtopic, "Initial message from "+WiFiSettings.hostname); 
    
    display_big("Ready !", TFT_RED);
    delay(1000);
    display_config(mqtt_broker,mqtt_Pubtopic,mqtt_Subtopic);
    delay(1000);
}

void loop() {
    // Your loop code here
    // Serial.println("In main loop");
    
    connect_mqtt();
    mqtt.loop();

    if (button(PIN_BUTTON_1)) 
        { // check if upper btn is pressed and invoke Wifi settings portal
             Serial.println("Btn 1 in main loop, entering wifi config portal mode");
             display_lines(T.portal_instructions[0], TFT_WHITE, TFT_BLUE);
             WiFiSettings.portal();
        }
     if (button(PIN_BUTTON_2)) 
        { // check if upper btn is pressed and invoke Wifi settings portal
            mqtt_template.replace("{}", WiFiSettings.hostname);
            Serial.printf("Button-2 press detected, pubbing : %s now..\n",mqtt_template);
            // display_big("Btn press"); // Show keypress on Oled
            MQTTpub(mqtt_Pubtopic, mqtt_template);  // Pub the MQTT message 
            delay(2000);
            
        }    
    delay(10);
}