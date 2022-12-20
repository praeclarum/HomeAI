#include "Api.h"
#include "Config.h"
#include "Thermometer.h"
#include "Knob.h"
#include "Display.h"

#define HEATER_HYSTERESIS_CELSIUS (1.0f * 5.0f/9.0f)

static bool isHeaterOn = false;
static unsigned long lastReadMillis = 0;
static float lastTargetCelsius = 0;
static float lastCelsius = 0;

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();

  displaySetup();
  thermometerSetup();
  knobSetup();
  apiSetup();
}

void loop() {

  const auto nowMillis = millis();

  const bool shouldRead = lastReadMillis == 0 || ((nowMillis - lastReadMillis) > 5 * 60 * 1000);

  const float currentCelsius = thermometerReadCelsius();
  lastCelsius = currentCelsius;
  
  if (shouldRead) {
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

    lastReadMillis = millis();
  }

  displayUpdate(currentCelsius, lastTargetCelsius, isHeaterOn);

  delay(100);
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

