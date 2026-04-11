#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

Preferences prefs;


const int PIN_OPTO_IN    = 5;   
const int PIN_TOUCH_IN   = 6;   
const int PIN_DIMMER_OUT = 7;   


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


bool isReady = false;           
unsigned long powerOnTime = 0;  
int opMode = 1;                 // 0: BREW, 1: MANUAL
String currentProfileName = "Basic"; 
bool isProcessing = false;      
unsigned long brewStartTime = 0; 


int preInfusionPower;      
int preInfusionTime;    
int pauseTime;
unsigned long rampUpDuration; 
unsigned long warmupLimit = 300000;

WebServer server(80);
IPAddress local_IP(192, 168, 0, 119);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

const char* ssid = "Your SSID";
const char* password = "password";


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
  analogWrite(PIN_DIMMER_OUT, 0);
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
    analogWrite(PIN_DIMMER_OUT, preInfusionPower);
  }

  // Phase 2: Pause
  unsigned long pauseStart = millis();
  while(millis() - pauseStart < pauseTime) {
    if(digitalRead(PIN_OPTO_IN) == LOW) break;
    updateBrewDisplay("PAUSING...", brewStartTime);
    analogWrite(PIN_DIMMER_OUT, 0);
  }

  // Phase 3: Extraction (Ramp up & Main)
  unsigned long rampStartTime = millis();
  while(digitalRead(PIN_OPTO_IN) == HIGH) {
    unsigned long now = millis();
    int currentPower = (now - rampStartTime < rampUpDuration) 
                       ? map(now - rampStartTime, 0, rampUpDuration, preInfusionPower, 255) 
                       : 255;
    analogWrite(PIN_DIMMER_OUT, currentPower);
    updateBrewDisplay("EXTRACTING: ", brewStartTime);
    
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
        for(int i=0; i<=255; i+=15) {
          analogWrite(PIN_DIMMER_OUT, i);
          delay(10);
        }
      }
    }

    if (opMode == 1) {
      analogWrite(PIN_DIMMER_OUT, 255);
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
  if (isReady) {
    display.print("READY!");
  } else {
    long remain = (warmupLimit - (millis() - powerOnTime)) / 1000;
    if (remain < 0) remain = 0;
    display.print("HEAT "); display.print(remain); display.print("s");
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
  if (doc.containsKey("prePower")) preInfusionPower = int(255 * doc["prePower"].as<int>() / 100);
  if (doc.containsKey("preTime"))  preInfusionTime  = 1000 * doc["preTime"].as<int>();
  if (doc.containsKey("pause"))    pauseTime        = 1000 * doc["pause"].as<int>();
  if (doc.containsKey("ramp"))     rampUpDuration   = 1000 * doc["ramp"].as<int>();
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
  Serial.begin(115200);
  powerOnTime = millis();
  
  pinMode(PIN_OPTO_IN, INPUT); 
  pinMode(PIN_TOUCH_IN, INPUT);
  pinMode(PIN_DIMMER_OUT, OUTPUT);
  analogWrite(PIN_DIMMER_OUT, 0);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);
  display.clearDisplay();

  prefs.begin("gaggia", false);
  preInfusionPower = prefs.getInt("prePower", 100);
  preInfusionTime  = prefs.getInt("preTime", 5000);
  pauseTime        = prefs.getInt("pauseTime", 3000);
  rampUpDuration   = prefs.getULong("rampUp", 2000);
  currentProfileName = prefs.getString("profName", "Classic");
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