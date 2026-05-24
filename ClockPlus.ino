#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// EEPROM address definition
#define EEPROM_ADDR_HOUR   0
#define EEPROM_ADDR_MINUTE 1
#define EEPROM_ADDR_SECOND 2
#define EEPROM_ADDR_FLAG   3

// Time variables
int hours = 0, minutes = 0, seconds = 0;
unsigned long lastTick = 0;
bool timeReceived = false;

// Smoke detection related variables
#define MQ2_AO_PIN A0       // MQ-2
#define BUTTON_PIN 3        //  Button(PULLUP)
#define SMOKE_THRESHOLD 62.714  // 0-1023
#define LED_PIN 8           // LED
#define BUZZER_PIN 12

bool smokeTriggered = false;      
int recordHour = 0, recordMinute = 0, recordSecond = 0;  
bool buttonPressed = false;        
bool showingRecord = false;        
unsigned long lastButtonTime = 0;  // For button debouncing
const int debounceDelay = 50;      

// smoke level related variables
int smokeLevel = 0;                // current level (0-1023)
int smokePercent = 0;              // level -> percent
unsigned long lastSerialSend = 0;  // Last serial transmission time
const int serialSendInterval = 2500; // Send serial data every 2500ms

void setup() {
  Serial.begin(9600);
  
  // Initialize pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); 
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    for(;;);
  }
  display.clearDisplay();
  
  // Read the last saved time from EEPROM
  if(EEPROM.read(EEPROM_ADDR_FLAG) == 0xAA) {
    hours = EEPROM.read(EEPROM_ADDR_HOUR);
    minutes = EEPROM.read(EEPROM_ADDR_MINUTE);
    seconds = EEPROM.read(EEPROM_ADDR_SECOND);
    timeReceived = true;
    lastTick = millis();
    
    // First display recovery prompt
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 28);
    display.println("Time Restored!");
    display.display();
    delay(1500);
    
    // Clear screen and force time display
    display.clearDisplay();
    display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    display.drawLine(0, 18, SCREEN_WIDTH, 18, SSD1306_WHITE);
    display.setTextSize(2);
    display.setCursor(20, 28);
    if(hours < 10) display.print("0");
    display.print(hours);
    display.print(":");
    if(minutes < 10) display.print("0");
    display.print(minutes);
    display.print(":");
    if(seconds < 10) display.print("0");
    display.print(seconds);
    display.setTextSize(1);
    display.setCursor(30, 52);
    display.print("Level: --%");
    display.display();
  } else {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(20, 28);
    display.println("Waiting for PC...");
    display.display();
  }
  
  Serial.println("Smoke Sensor Started");
  Serial.println("Time, SmokeLevel, SmokePercent");
}

void loop() {
  // 1. Listen for time synchronization from the computer
  if(Serial.available() > 0) {
    String timeStr = Serial.readStringUntil('\n');
    timeStr.trim();
    if(sscanf(timeStr.c_str(), "%d:%d:%d", &hours, &minutes, &seconds) == 3) {
      timeReceived = true;
      lastTick = millis();
      smokeTriggered = false;
      showingRecord = false;
      
      // Turn off the LED when resetting
      digitalWrite(LED_PIN, LOW);
      
      // Save time to EEPROM
      EEPROM.write(EEPROM_ADDR_HOUR, hours);
      EEPROM.write(EEPROM_ADDR_MINUTE, minutes);
      EEPROM.write(EEPROM_ADDR_SECOND, seconds);
      EEPROM.write(EEPROM_ADDR_FLAG, 0xAA);
      
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(35, 28);
      display.println("Synced!");
      display.display();
      delay(1000);
      
      Serial.println("Time Synced!");
    }
  }
  
  // 2. Read smoke concentration in real time
  if(timeReceived) {
    smokeLevel = analogRead(MQ2_AO_PIN);
    smokePercent = map(smokeLevel, 0, 1023, 0, 100);
    
    // 3. Send real-time smoke level to serial monitor
    if(millis() - lastSerialSend >= serialSendInterval) {
      lastSerialSend = millis();
      
      Serial.print(hours);
      Serial.print(":");
      if(minutes < 10) Serial.print("0");
      Serial.print(minutes);
      Serial.print(":");
      if(seconds < 10) Serial.print("0");
      Serial.print(seconds);
      Serial.print(", ");
      Serial.print(smokeLevel);
      Serial.print(", ");
      Serial.print(smokePercent);
      Serial.println("%");
    }
    
    // 4. Smoke detection: record the time once the threshold is exceeded (only record the first time)
    if(!smokeTriggered && smokeLevel > SMOKE_THRESHOLD) {
      smokeTriggered = true;
      recordHour = hours;
      recordMinute = minutes;
      recordSecond = seconds;
      
      // When smoke is triggered, the LED turns on and stays on
      digitalWrite(LED_PIN, HIGH);
      tone(BUZZER_PIN, 1000);

      Serial.println("!!! SMOKE DETECTED !!!");
      Serial.print("Recorded at: ");
      Serial.print(recordHour);
      Serial.print(":");
      if(recordMinute < 10) Serial.print("0");
      Serial.print(recordMinute);
      Serial.print(":");
      if(recordSecond < 10) Serial.print("0");
      Serial.println(recordSecond);
      Serial.print("Smoke Level: ");
      Serial.print(smokeLevel);
      Serial.print(" (");
      Serial.print(smokePercent);
      Serial.println("%)");
    }
  }
  
  // 5. Button detection (switch display mode)
  int buttonState = digitalRead(BUTTON_PIN);
  if(buttonState == LOW && !buttonPressed && (millis() - lastButtonTime > debounceDelay)) {
    buttonPressed = true;
    lastButtonTime = millis();
    
    if(smokeTriggered) {
      showingRecord = !showingRecord;
      
      // When switching to record display mode, the LED turns off
      if(showingRecord) {
        digitalWrite(LED_PIN, LOW);  // Checked the record, turn off the LED
        noTone(BUZZER_PIN);
      }
      
      Serial.print("Display mode: ");
      Serial.println(showingRecord ? "Record Time" : "Normal Time");
    }
  }
  if(buttonState == HIGH) {
    buttonPressed = false;
  }
  
  // 6. Update time (once per second)
  if(timeReceived) {
    if(millis() - lastTick >= 1000) {
      seconds++;
      if(seconds >= 60) {
        seconds = 0;
        minutes++;
        if(minutes >= 60) {
          minutes = 0;
          hours++;
          if(hours >= 24) hours = 0;
        }
      }
      lastTick = millis();
      
      
      
      updateDisplay();
    }
  }
}

void updateDisplay() {
  display.clearDisplay();
  
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.drawLine(0, 18, SCREEN_WIDTH, 18, SSD1306_WHITE);
  
  display.setTextSize(1);
  
  // If no time is received, display a waiting prompt.
  if(!timeReceived) {
    display.setCursor(25, 28);
    display.println("Waiting for PC...");
    display.display();
    return;
  }
  
  if(smokeTriggered && showingRecord) {
    display.setCursor(25, 4);
    display.println("Smoke Recorded!");
    
    display.setTextSize(2);
    display.setCursor(20, 28);
    if(recordHour < 10) display.print("0");
    display.print(recordHour);
    display.print(":");
    if(recordMinute < 10) display.print("0");
    display.print(recordMinute);
    display.print(":");
    if(recordSecond < 10) display.print("0");
    display.print(recordSecond);
    
    display.setTextSize(1);
    display.setCursor(30, 52);
    display.print("Now: ");
    display.print(smokePercent);
    display.print("%");
  } 
  else if(smokeTriggered && !showingRecord) {
    display.setCursor(35, 4);
    display.println("Normal Time");
    
    display.fillRect(100, 2, 24, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setCursor(105, 5);
    display.println("REC");
    display.setTextColor(SSD1306_WHITE);
    
    display.setTextSize(2);
    display.setCursor(20, 28);
    if(hours < 10) display.print("0");
    display.print(hours);
    display.print(":");
    if(minutes < 10) display.print("0");
    display.print(minutes);
    display.print(":");
    if(seconds < 10) display.print("0");
    display.print(seconds);
    
    display.setTextSize(1);
    display.setCursor(15, 52);
    display.print("Smoke:");
    display.print(smokePercent);
    display.print("%");
    
    display.setCursor(85, 52);
    display.print("Btn:REC");
  }
  else {
    display.setCursor(35, 4);
    display.println("Smoke Sensor");
    
    display.setTextSize(2);
    display.setCursor(20, 28);
    if(hours < 10) display.print("0");
    display.print(hours);
    display.print(":");
    if(minutes < 10) display.print("0");
    display.print(minutes);
    display.print(":");
    if(seconds < 10) display.print("0");
    display.print(seconds);

    
    display.setTextSize(1);
    display.setCursor(30, 52);
    display.print("Level: ");
    display.print(smokePercent);
    display.print("%");
  }
  
  display.display();
}
