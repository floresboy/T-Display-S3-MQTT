#include "Arduino.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include "img_logo.h"
#include "pin_config.h"
#include <SPIFFS.h>
#include <WiFiSettings.h>




#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5,0,0)
#error  "The current version is not supported for the time being, please use a version below Arduino ESP32 3.0"
#endif

/* The product now has two screens, and the initialization code needs a small change in the new version. The LCD_MODULE_CMD_1 is used to define the
 * switch macro. */
#define LCD_MODULE_CMD_1

TFT_eSPI tft = TFT_eSPI();

#define WAIT 1000
unsigned long targetTime = 0; // Used for testing draw times

#if defined(LCD_MODULE_CMD_1)
typedef struct {
    uint8_t cmd;
    uint8_t data[14];
    uint8_t len;
} lcd_cmd_t;

lcd_cmd_t lcd_st7789v[] = {
    {0x11, {0}, 0 | 0x80},
    {0x3A, {0X05}, 1},
    {0xB2, {0X0B, 0X0B, 0X00, 0X33, 0X33}, 5},
    {0xB7, {0X75}, 1},
    {0xBB, {0X28}, 1},
    {0xC0, {0X2C}, 1},
    {0xC2, {0X01}, 1},
    {0xC3, {0X1F}, 1},
    {0xC6, {0X13}, 1},
    {0xD0, {0XA7}, 1},
    {0xD0, {0XA4, 0XA1}, 2},
    {0xD6, {0XA1}, 1},
    {0xE0, {0XF0, 0X05, 0X0A, 0X06, 0X06, 0X03, 0X2B, 0X32, 0X43, 0X36, 0X11, 0X10, 0X2B, 0X32}, 14},
    {0xE1, {0XF0, 0X08, 0X0C, 0X0B, 0X09, 0X24, 0X2B, 0X22, 0X43, 0X38, 0X15, 0X16, 0X2F, 0X37}, 14},
};
#endif



// ****** KS code *******




bool button(int pin) {
    if (digitalRead(pin)) return false;
    unsigned long start = millis();
    while (!digitalRead(pin)) {
        if (millis() - start >= 50) Serial.println("In button, returning true");
        return(true);
    }
    return millis() - start >= 50;
}


void check_portalbutton() {
    if (button(PIN_BUTTON_1))  Serial.println("chk port but");
}



// Configuration via WiFiSettings
unsigned long   mqtt_interval;
String          mqtt_Pubtopic;
String          mqtt_Subtopic;
String          mqtt_template;
bool            wifi_enabled;
bool            mqtt_enabled;
int             max_failures;



void setup()
{
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    Serial.begin(115200);
    Serial.println("BTest app started");

    tft.begin();
    SPIFFS.begin(true);  // On first run, will format after failing to mount

// *****w wifi settings ******
    WiFiSettings.hostname = "HiveMQ-";
    WiFiSettings.language = LANGUAGE;

    WiFiSettings.connect(); // On failure it wil go into wifi portal mode, on succes it will con't.



#if defined(LCD_MODULE_CMD_1)
    for (uint8_t i = 0; i < (sizeof(lcd_st7789v) / sizeof(lcd_cmd_t)); i++) {
        tft.writecommand(lcd_st7789v[i].cmd);
        for (int j = 0; j < (lcd_st7789v[i].len & 0x7f); j++) {
            tft.writedata(lcd_st7789v[i].data[j]);
        }

        if (lcd_st7789v[i].len & 0x80) {
            delay(120);
        }
    }
#endif


#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5,0,0)
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    ledcWrite(0, 255);
#else
    ledcAttach(PIN_LCD_BL, 200, 8);
    ledcWrite(PIN_LCD_BL, 255);
#endif
}



void SetTextOnDIsplay(void)
{
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);

    tft.drawString(" kamiel ", 0, 0, 2);
    tft.drawString("789:;<=>?@ABCDEFGHIJKL", 0, 16, 2);
    tft.drawString("MNOPQRSTUVWXYZ[\\]^_`", 0, 32, 2);
    tft.drawString("abcdefghijklmnopqrstuvw", 0, 48, 2);
}

void loop()
{
    targetTime = millis();
   
    /* First we test them with a background colour set */
   //  Serial.println("Gen text on display");
    SetTextOnDIsplay();
 
    if (button(PIN_BUTTON_1)) 
        { // check if upper btn is pressed and invoke Wifi settings portal
             Serial.println("Btn 1 in main loop, entering wifi config portal mode");
             WiFiSettings.portal();
        }

    if (button(PIN_BUTTON_2)) 
        { // check if upper btn is pressed and invoke Wifi settings portal
             Serial.println("Btn 2 in main loop");
        }

    delay(10);
    
    // Serial.println("clear display");
    tft.fillScreen(TFT_BLACK);
    delay(10);
    
    
}
