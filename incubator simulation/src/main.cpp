#include <Wire.h>
#include "RTClib.h"
#include <DHT.h>

//READ THIS MANUAL FIRST!!//
// blueLED= HUMIDIFIER
// redLED= HEATER
// blueLED= SERVOMOTOR
// greenLED=FAN

// ---------------- Pin Definitions ----------------
#define DHTPIN 4
#define DHTTYPE DHT22

#define HEATER_PIN 18
#define HUMIDIFIER_PIN 19
#define EGG_TURN_PIN 23
#define FAN_PIN 25
#define BUZZER_PIN 22  // Alarm

// ---------------- Sensor & RTC Setup ----------------
DHT dht(DHTPIN, DHTTYPE);
RTC_DS3231 rtc;

// ---------------- Hysteresis Thresholds ----------------
const float TEMP_LOW = 37.3;
const float TEMP_HIGH = 37.7;
const float HUMIDITY_LOW = 50.0;
const float HUMIDITY_HIGH = 55.0;

// ---------------- Egg Turning ----------------
// Simulation: 10 seconds = 1 "day" for testing
const unsigned long TURN_INTERVAL_SIM = 10000;
unsigned long lastTurn = 0;
bool turnedThisInterval = false;

// ---------------- Day Counter ----------------
int incubationDay = 1;

void setup() {
  Serial.begin(115200);
  dht.begin();

  // ---------------- Setup Relays ----------------
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, HIGH); // OFF initially (active LOW)

  pinMode(HUMIDIFIER_PIN, OUTPUT);
  digitalWrite(HUMIDIFIER_PIN, HIGH); // OFF initially

  pinMode(EGG_TURN_PIN, OUTPUT);
  digitalWrite(EGG_TURN_PIN, LOW); // Motor OFF initially

  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, HIGH); // OFF initially (active LOW)

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // ---------------- RTC Initialization ----------------
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time to compile time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("Phase 5: Incubator control started");
}

void loop() {
  // ---------------- Read DHT22 ----------------
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Sensor error");
    delay(2000);
    return;
  }

  Serial.print("Temperature: "); Serial.print(temperature); Serial.print(" C, ");
  Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");

  // ---------------- Heater Control ----------------
  if (temperature < TEMP_LOW) {
    digitalWrite(HEATER_PIN, HIGH);  // Heater ON
    Serial.println("Heater ON");
  } else if (temperature > TEMP_HIGH) {
    digitalWrite(HEATER_PIN, LOW);   // Heater OFF
    Serial.println("Heater OFF");
  }

  // ---------------- Humidifier Control ----------------
  if (humidity < HUMIDITY_LOW) {
    digitalWrite(HUMIDIFIER_PIN, HIGH);  // Humidifier ON
    Serial.println("Humidifier ON");
  } else if (humidity > HUMIDITY_HIGH) {
    digitalWrite(HUMIDIFIER_PIN, LOW);   // Humidifier OFF
    Serial.println("Humidifier OFF");
  }

  // ---------------- Fan Control ----------------
  // Fan runs continuously to circulate air
  digitalWrite(FAN_PIN, HIGH);  // ON
  Serial.println("Fan ON");

  // ---------------- Egg Turning ----------------
  unsigned long currentMillis = millis();
  if (currentMillis - lastTurn >= TURN_INTERVAL_SIM) {
    if (!turnedThisInterval) {
      digitalWrite(EGG_TURN_PIN, HIGH); // Motor ON
      delay(5000);                       // Rotate 5 sec
      digitalWrite(EGG_TURN_PIN, LOW);   // Motor OFF
      lastTurn = currentMillis;
      turnedThisInterval = true;
      Serial.println("Egg turned");
      incubationDay++; // Increment day (for simulation)
      Serial.print("Incubation Day: "); Serial.println(incubationDay);
    }
  } else {
    turnedThisInterval = false;
  }

  // ---------------- Alarm ----------------
  if (temperature < 36 || temperature > 39 || humidity < 45 || humidity > 70) {
    digitalWrite(BUZZER_PIN, HIGH);  // Alert ON
  } else {
    digitalWrite(BUZZER_PIN, LOW);   // Safe
  }

  Serial.println("------------------------");
  delay(2000);
}