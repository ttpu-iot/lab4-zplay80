

#define BLYNK_FIRMWARE_VERSION "0.1.1"

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6uCkbn7qu"
#define BLYNK_TEMPLATE_NAME "iot25 lab4"
#define BLYNK_AUTH_TOKEN "-4u-dEEvjLNlLMSy_kcEO97EmqdyUvEM"


/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
// #define BLYNK_DEBUG


#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>

#include <BlynkSimpleEsp32.h>
// #include <BlynkSimpleEsp32_SSL.h>

#include <HTTPClient.h>
#include <Update.h>

#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>



//----------------------------------------------
// GLOBAL VARIABLES and CONSTANTS
// your code here
const int RED_PIN = 26;
const int GREEN_PIN = 27;
const int BLUE_PIN = 14;
const int YELLOW_PIN = 12;

// LCD Configuration
hd44780_I2Cexp lcd;  // Auto-detect I2C address
const int LCD_COLS = 16;
const int LCD_ROWS = 2;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Wokwi-GUEST";
char pass[] = "";
// char ssid[] = "MaxPC";
// char pass[] = "polito2025";

//----------------------------------------------
// FUNCTIONS
BLYNK_WRITE(V0)
{   
  int value = param.asInt(); // Get value as integer
  digitalWrite(RED_PIN, value);
}

//----------------------------------------------
// BLYNK_WRITE lets you listen for Blynk OTA triggers
// --- OTA Handler using Update.h ---
BLYNK_WRITE(InternalPinOTA) {
    String url = param.asString();
    
    // Convert HTTPS to HTTP by removing the 's'
    // if (url.startsWith("https://")) {
    //     url.replace("https://", "http://");
    // }
    
    Serial.printf("OTA update requested: %s\n", url.c_str());
    Serial.println("\r\nStarting OTA update...");

    // Disconnect Blynk to not interfere with OTA process
    Blynk.disconnect();

    HTTPClient http;
    http.begin(url);

    // Collect MD5 header if available
    const char* headerkeys[] = { "x-MD5" };
    http.collectHeaders(headerkeys, sizeof(headerkeys)/sizeof(char*));

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("HTTP GET failed, error: %d\n", httpCode);
        http.end();
        Blynk.connect();
        return;
    }

    int contentLength = http.getSize();
    if (contentLength <= 0) {
        Serial.println("Content-Length not defined");
        http.end();
        Blynk.connect();
        return;
    }

    Serial.printf("Content-Length: %d bytes\n", contentLength);

    bool canBegin = Update.begin(contentLength);
    if (!canBegin) {
        Serial.println("Not enough space to begin OTA");
        http.end();
        Blynk.connect();
        return;
    }

    // Set MD5 hash if provided in headers
    if (http.hasHeader("x-MD5")) {
        String md5 = http.header("x-MD5");
        if (md5.length() == 32) {
            md5.toLowerCase();
            Serial.printf("Expected MD5: %s\n", md5.c_str());
            Update.setMD5(md5.c_str());
        }
    }

    WiFiClient* client = http.getStreamPtr();
    int written = Update.writeStream(*client);
    
    if (written != contentLength) {
        Serial.printf("OTA written %d / %d bytes\n", written, contentLength);
        Update.abort();
        http.end();
        Blynk.connect();
        return;
    }

    if (!Update.end()) {
        Serial.printf("Update error #%d\n", Update.getError());
        http.end();
        Blynk.connect();
        return;
    }

    if (!Update.isFinished()) {
        Serial.println("Update failed.");
        http.end();
        Blynk.connect();
        return;
    }

    Serial.println("=== Update successfully completed. Rebooting.");
    http.end();
    delay(100);
    ESP.restart();
}

//----------------------------------------------
// SETUP FUNCTION
void setup(void) 
{
    // Serial setup
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting setup...");

    // led setup
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);
    pinMode(YELLOW_PIN, OUTPUT);

    // Initialize LCD
    int status = lcd.begin(LCD_COLS, LCD_ROWS);
    if (status) {
        Serial.println("LCD initialization failed!");
        Serial.print("Status code: ");
        Serial.println(status);
        hd44780::fatalError(status);
    }
  
    Serial.println("LCD initialized successfully!");

    // Clear LCD and display initial message
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");
    delay(1000);

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);  
    // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "ny3.blynk.cloud", 80);
}


//----------------------------------------------
// LOOP FUNCTION
void loop(void) 
{
    Blynk.run();
}