#include "Api.h"

#define HEATER_HYSTERESIS_CELSIUS 1.0f

static bool isHeaterOn = false;

#include <TM1637Display.h>
#define DISPLAY_CLK_PIN 32
#define DISPLAY_DIO_PIN 25
static TM1637Display tm(DISPLAY_CLK_PIN, DISPLAY_DIO_PIN);

#include <DHT.h>
#define DHT_PIN 33
#define DHT_TYPE DHT22
static DHT dht(DHT_PIN, DHT_TYPE);

void tempSetup();
float tempRead();

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  tm.setBrightness(2);
  tm.showNumberDec(0);

  tempSetup();

  apiSetup();

}

static unsigned long lastReadMillis = 0;
static float lastTargetCelsius = 0;

void loop() {

  const auto nowMillis = millis();

  const bool shouldRead = lastReadMillis == 0 || ((nowMillis - lastReadMillis) > 5 * 60 * 1000);

  float currentCelsius = tempRead();
  
  if (shouldRead) {
    lastReadMillis = nowMillis;
    
    Serial.print("READ ");
    Serial.print(currentCelsius);
    Serial.print("C ");
    Serial.print(currentCelsius * 9.0f / 5.0f + 32.0f);
    Serial.println("F");

    if (apiPostTemperature(currentCelsius)) {
      float targetCelsius = 0.0;
      if (apiGetTargetTemperature(&targetCelsius)) {
        lastTargetCelsius = targetCelsius;
        Serial.print("TARGET ");
        Serial.print(targetCelsius);
        Serial.println("C ");
        if (apiPostTargetTemperature(targetCelsius)) {
          control(currentCelsius, targetCelsius);
        }
      }
    }
  }

  const auto currentF = int(currentCelsius*9.0f/5.0f + 32.0f + 0.5f);
  const auto targetF = int(lastTargetCelsius*9.0f/5.0f + 32.0f + 0.5f);

  tm.showNumberDecEx(currentF, 0b01000000, true, 2, 0);
  tm.showNumberDecEx(targetF, 0b01000000, true, 2, 2);
  delay(1000);
}

void control(float currentCelsius, float targetCelsius)
{
  bool heaterShouldBeOn = false;
  
  if (isHeaterOn) {
    // Turn off if current temp greater than setpoint plus hysteresis
    const auto heaterShouldBeOff = currentCelsius > targetCelsius + HEATER_HYSTERESIS_CELSIUS;
    heaterShouldBeOn = !heaterShouldBeOff;
  }
  else {
    // Turn on if current temp is less then setpoint minus hysteresis
    heaterShouldBeOn = currentCelsius < targetCelsius - HEATER_HYSTERESIS_CELSIUS;
  }
  if (apiPostHeaterOn(heaterShouldBeOn)) {
    isHeaterOn = heaterShouldBeOn;
    // Actually turn on the relay
    // digitalWrite(HEATER_RELAY_PIN, isHeaterOn ? HIGH : LOW);
  }
}

void tempSetup() {
  dht.begin();
}

float tempRead() {
  float celsius = dht.readTemperature();
  return celsius;
}