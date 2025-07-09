// Sistema Anti-Robo para Motocicleta: Apantalla-Ratas™ 3077

#include <LiquidCrystal.h>

// LCD pin setup: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(10, 11, 12, 13, 22, 23);

// Pin assignments
const int pirPin = 2;                   // PIR sensor OUT
const int buzzerPin = 3;                // Buzzer control
const int alarmLeds[] = {4, 5, 8, 9};   // Dissuasive LEDs
const int buttonPin = 6;                // Push button toggle
const int statusLedPin = 7;             // Armed/Disarmed status LED

// System states
bool systemArmed = false;
bool alarmActive = false;

// Button debounce
bool previousButtonState = LOW;
unsigned long lastDebounceTime = 0;
const int debounceDelay = 200; // ms

// Alarm duration
const int alarmDuration = 5000; // ms

void setup() {
  
  // Console debugging
  Serial.begin(9600);

  // Display settings
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Apantalla Ratas");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();

  pinMode(pirPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(statusLedPin, OUTPUT);

  for (int i = 0; i < sizeof(alarmLeds) / sizeof(alarmLeds[0]); i++) {
    pinMode(alarmLeds[i], OUTPUT);
    digitalWrite(alarmLeds[i], LOW);
  }

  digitalWrite(buzzerPin, LOW);
  digitalWrite(statusLedPin, LOW);
}

// Confirm movement (avoid false positives)
bool isMovementConfirmed() {
  int count = 0;
  for (int i = 0; i < 5; i++) {
    if (digitalRead(pirPin) == HIGH) {
      count++;
    }
    delay(50);
  }
  return (count >= 3);
}

void loop() {
  // handle button toggle with debounce
  bool buttonState = digitalRead(buttonPin);
  if (buttonState != previousButtonState && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    if (buttonState == HIGH) {
      systemArmed = !systemArmed;
      Serial.print("system in now ");
      Serial.println(systemArmed ? "ARMED" : "DISARMED");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("System Status:");
      lcd.setCursor(0, 1);
      lcd.print(systemArmed ? "ARMED" : "DISARMED");
    }
    previousButtonState = buttonState;
  }

  // Show armed status
  if (!systemArmed) {
    digitalWrite(statusLedPin, LOW); // off
  } else if (!alarmActive) {
    // blinking when armed
    digitalWrite(statusLedPin, HIGH);
    delay(100);
    digitalWrite(statusLedPin, LOW);
    delay(900);
  }

  // Trigger alarm if movement detected
  if (systemArmed && isMovementConfirmed()) {
    alarmActive = true;
    Serial.println("Intruder detected - alarm triggered!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! INTRUDER !!!");
    lcd.setCursor(0, 1);
    lcd.print("ALARM TRIGGERED");

    unsigned long startTime = millis();
    while (millis() - startTime < alarmDuration) {
      digitalWrite(buzzerPin, HIGH);

      // Fancy LED flashing
      for (int i = 0; i < sizeof(alarmLeds) / sizeof(alarmLeds[0]); i++) {
        digitalWrite(alarmLeds[i], HIGH);
        delay(70);
        digitalWrite(alarmLeds[i], LOW);
      }

      digitalWrite(statusLedPin, HIGH); // solid ON while alarm active
    }

    digitalWrite(buzzerPin, LOW);
    alarmActive = false;
    Serial.println("Alarm ended. Back to monitoring");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Back to standby");
    lcd.setCursor(0, 1);
    lcd.print("Monitoring...");

    delay(2000); // cooldown
  }
}