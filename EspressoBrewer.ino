#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "rbdimmerESP32.h"

Preferences prefs;


// ===== RBDimmer =====
#define ZERO_CROSS_PIN   4   // PIN_ZERO_CROSS(ZC)
#define DIMMER_PIN       7   // PIN_DIMMER_OUT (TRIAC)

#define PHASE_NUM        0   

rbdimmer_channel_t* dimmer_channel = NULL;

// ===== Pin setting =====
const int PIN_OPTO_IN    = 5;   // Optocoupler signal pin
const int PIN_TOUCH_IN   = 6;   // Touch sensor signal pin
const int PIN_DIMMER_OUT = 7;   // Dimmer signal pin

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool isReady = false;           
unsigned long powerOnTime = 0;  
int opMode = 1;                 // 0: BREW, 1: MANUAL
String currentProfileName = "Basic"; 
bool isProcessing = false;      
unsigned long brewStartTime = 0; 

int preInfusionPower;                // 0-100%   
int preInfusionTime;    
int pauseTime;
int maxPower;
unsigned long rampUpDuration; 
unsigned long warmupLimit = 300000;  // Time to temp(PID) stabilizization (5min.)

WebServer server(80);
IPAddress local_IP(192, 168, 0, XXX);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

const char* ssid = "Your SSID";
const char* password = "password";


void setDimmerLevel(int percent) {
  if (dimmer_channel != NULL) {
    rbdimmer_set_level(dimmer_channel, percent);
  }
}

void displayStatus(String msg, String bigMsg) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); display.setTextSize(1);
  display.println(msg);
  display.setCursor(0, 16); display.setTextSize(2);
  display.println(bigMsg);
  display.display();
}

void updateBrewDisplay(String status, unsigned long startTime) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); display.setTextSize(1);
  display.print(status);
  display.setCursor(0, 16); display.setTextSize(2);
  display.print((millis() - startTime) / 1000);
  display.print(" SEC");
  display.display();
}

void showFinalTime(unsigned long startTime) {
  setDimmerLevel(0); 
  unsigned long total = (millis() - startTime) / 1000;
  displayStatus("TOTAL TIME", String(total) + " SEC");
  delay(5000); 
  isProcessing = false;
}

void checkReadyStatus() {
  if (millis() - powerOnTime > warmupLimit) isReady = true;
  else isReady = false;
}

void runBrewCycle() {
  brewStartTime = millis(); 
  
  // Phase 1: Pre-Infusion
  while(millis() - brewStartTime < preInfusionTime) {
    if(digitalRead(PIN_OPTO_IN) == LOW) break;
    updateBrewDisplay("INFUSING...", brewStartTime);
    setDimmerLevel(preInfusionPower); 
    yield();
  }

  // Phase 2: Pause
  unsigned long pauseStart = millis();
  while(millis() - pauseStart < pauseTime) {
    if(digitalRead(PIN_OPTO_IN) == LOW) break;
    updateBrewDisplay("PAUSING...", brewStartTime);
    setDimmerLevel(0);
    yield();
  }

  // Phase 3: Extraction (Ramp up & Main)
  unsigned long rampStartTime = millis();
  while(digitalRead(PIN_OPTO_IN) == HIGH) {
    unsigned long now = millis();
    int currentPercent = (now - rampStartTime < rampUpDuration) 
                       ? map(now - rampStartTime, 0, rampUpDuration, preInfusionPower, 100) 
                       : 100;
    setDimmerLevel(currentPercent);
    updateBrewDisplay("EXTRACTING: ", brewStartTime);
    yield();
    
    isReady = false;
    powerOnTime = millis();
    warmupLimit = 120000; 
  }
  
  showFinalTime(brewStartTime);
}

void handleExtraction() {
  bool optoState = digitalRead(PIN_OPTO_IN); 

  if (optoState == HIGH) { 
    if (!isProcessing) { 
      brewStartTime = millis(); 
      isProcessing = true; 
      if (opMode == 1) { 
        displayStatus("MANUAL START", ""); 
        for(int i=0; i<=100; i+=15) {
          setDimmerLevel(i);
          delay(10);
        }
      }
    }

    if (opMode == 1) {
      setDimmerLevel(100);
      updateBrewDisplay("PUMPING!", brewStartTime);
    } else {
      runBrewCycle();
    }
  } 
  else if (isProcessing) { 
    showFinalTime(brewStartTime);
  }
}

void handleTouch() {
  static bool lastTouchState = false;
  static unsigned long lastDebounceTime = 0;
  bool currentTouch = digitalRead(PIN_TOUCH_IN);

  if (currentTouch && !lastTouchState && (millis() - lastDebounceTime > 300)) {
    opMode = (opMode == 1) ? 0 : 1;
    lastDebounceTime = millis();
    Serial.printf("Mode changed to: %s\n", opMode == 1 ? "MANUAL" : "BREW");
  }
  lastTouchState = currentTouch;
}

void updateOLED() {
  if (isProcessing) return; 

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0, 0); display.setTextSize(1);
  if (opMode == 0) display.print("BREW: " + currentProfileName);
  else display.print("[ MANUAL MODE ]");

  display.setCursor(0, 16); display.setTextSize(2); 
  if (opMode == 0) {
    if (isReady) {display.print("READY!");}
    else {
      long remain = (warmupLimit - (millis() - powerOnTime)) / 1000;
      if (remain < 0) remain = 0;
      display.print("HEAT "); display.print(remain); display.print("s");
    }
  }
  display.display();
}

void savePreferences() {
  prefs.begin("gaggia", false);
  prefs.putInt("prePower", preInfusionPower);
  prefs.putInt("preTime", preInfusionTime);
  prefs.putInt("pauseTime", pauseTime);
  prefs.putULong("rampUp", rampUpDuration);
  prefs.putString("profName", currentProfileName);
  prefs.end();
}

void handleSettings() {
  if (server.hasArg("plain") == false) return;
  StaticJsonDocument<512> doc;
  deserializeJson(doc, server.arg("plain"));
  
  if (doc.containsKey("name")) currentProfileName = doc["name"].as<String>();
  if (doc.containsKey("prePower")) preInfusionPower = doc["prePower"].as<int>();
  if (doc.containsKey("preTime"))  preInfusionTime  = 1000 * doc["preTime"].as<int>();
  if (doc.containsKey("pause"))    pauseTime        = 1000 * doc["pause"].as<int>();
  if (doc.containsKey("ramp"))     rampUpDuration   = 1000 * doc["ramp"].as<int>();
  
  preInfusionPower = constrain(preInfusionPower, 0, 100);
  
  savePreferences();
  server.send(200, "application/json", "{\"result\":\"OK\"}");
}

void handleStatus() {
  StaticJsonDocument<256> doc;
  doc["isReady"] = isReady;
  doc["opMode"] = (int)opMode;
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void setup() {
  delay(1000);  // capacitor charging

  Serial.begin(115200);
  powerOnTime = millis();
  
  pinMode(PIN_OPTO_IN, INPUT_PULLUP); 
  pinMode(PIN_TOUCH_IN, INPUT_PULLUP);
  pinMode(PIN_DIMMER_OUT, OUTPUT);
  
  rbdimmer_init();
  rbdimmer_register_zero_cross(ZERO_CROSS_PIN, PHASE_NUM, 0);
  
  //RBDIMMER_CURVE_LINEAR: The light output changes directly with the input signal.
  //RBDIMMER_CURVE_LOGARITHMIC: Changes in the dimming signal happen slower at low level
  //                            and faster at the brighter end     
  rbdimmer_config_t dimmer_config = {
    .gpio_pin = DIMMER_PIN,
    .phase = PHASE_NUM,
    .initial_level = 0,
    //.curve_type = RBDIMMER_CURVE_LINEAR
    .curve_type = RBDIMMER_CURVE_LOGARITHMIC
  };
  
  rbdimmer_create_channel(&dimmer_config, &dimmer_channel);
  setDimmerLevel(0);
  
  Serial.println("RBDimmer initialized successfully!");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);
  display.clearDisplay();
  display.display();

  prefs.begin("gaggia", false);
  preInfusionPower   = prefs.getInt("prePower", 25);            // defualt: 25%
  preInfusionTime    = prefs.getInt("preTime", 10000);          // defualt: 10 Sec
  pauseTime          = prefs.getInt("pauseTime", 15000);        // defualt: 15 Sec
  maxPower           = prefs.getInt("maxPower", 100);           // defualt: 100%
  rampUpDuration     = prefs.getULong("rampUp", 3000);          // defualt: 3 Sec
  currentProfileName = prefs.getString("profName", "Basic");
  prefs.end();

  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  
  server.on("/set", HTTP_POST, handleSettings);
  server.on("/status", HTTP_GET, handleStatus);
  server.begin();

  opMode = 1; 
  isProcessing = false;
}

void loop() {
  server.handleClient();
  handleTouch();
  checkReadyStatus();

  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 500) {
    updateOLED();
    lastDisplayUpdate = millis();
  }
  handleExtraction();
}
