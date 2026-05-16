#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 6);

const int potPin = A0;
const int pwmPin = 9;
const int tachPin = 2;

volatile unsigned long pulseInterval = 0;
volatile unsigned long lastPulseTime = 0;

unsigned long lastSerialTime = 0;
unsigned long lastLcdTime = 0;  // Add timer for LCD update

void senseRotation() {
  unsigned long currentTime = micros();
  if (currentTime - lastPulseTime > 1000) { 
    pulseInterval = currentTime - lastPulseTime;
    lastPulseTime = currentTime;
  }
}

void setup() {
  // Increase communication speed to 115200bps
  Serial.begin(115200); 

  lcd.begin(16, 2);
  lcd.print("Hello Fan Controller");
  delay(1000);
  lcd.clear();
  
  pinMode(pwmPin, OUTPUT);
  pinMode(tachPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(tachPin), senseRotation, FALLING);
}

void loop() {
  // --- 1. Fan speed control (executed in each loop) ---
  int sensorValue = analogRead(potPin);
  int outputValue = (sensorValue < 20) ? 0 : map(sensorValue, 20, 1023, 0, 255);
  analogWrite(pwmPin, outputValue);
  
  // RPM calculation
  unsigned long _interval;
  unsigned long _lastPulse;
  noInterrupts();
  _interval = pulseInterval;
  _lastPulse = lastPulseTime;
  interrupts();

  unsigned long rpm = 0;
  if (micros() - _lastPulse > 1000000) {
    rpm = 0;
  } else if (_interval > 0) {
    rpm = 30000000 / _interval;
  }

  // --- 2. Serial output (every 50ms) ---
  if (millis() - lastSerialTime >= 50) {
    // Send in label:value format for easy parsing by Python
    Serial.print("PWM:");
    Serial.print(outputValue);
    Serial.print(",RPM:");
    Serial.println(rpm);
    lastSerialTime = millis();
  }

  // --- 3. LCD display (every 300ms) ---
  // Limit update frequency to make it readable for humans, as LCD updates are slow
  if (millis() - lastLcdTime >= 300) {
    lcd.setCursor(0, 0);
    lcd.print("RPM: ");
    lcd.print(rpm);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("PWM: ");
    lcd.print(outputValue); 
    lcd.print("    ");
    lastLcdTime = millis();
  }
}