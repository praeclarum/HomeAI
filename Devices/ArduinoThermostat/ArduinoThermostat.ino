#include "Api.h"

#define HEATER_HYSTERESIS_CELSIUS 1.0f

#include "DHT.h"
#define DHTPIN 33
#define DHTTYPE DHT22
static DHT dht(DHTPIN, DHTTYPE);

void tempSetup();
float tempRead();

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  tempSetup();

  apiSetup();
}

void loop() {
  
  float currentCelsius = tempRead();
  Serial.print("READ ");
  Serial.print(currentCelsius);
  Serial.print("C ");
  Serial.print(currentCelsius * 9.0f / 5.0f + 32.0f);
  Serial.println("F");

  if (apiPostTemperature(currentCelsius)) {
    float targetCelsius = 0.0;
    if (apiGetTargetTemperature(&targetCelsius)) {
      Serial.print("TARGET ");
      Serial.print(targetCelsius);
      Serial.print("C ");
      if (apiPostTargetTemperature(targetCelsius)) {
        control(currentCelsius, targetCelsius);
      }
    }
  }

  delay(30 * 1000);  
}

void control(float currentCelsius, float targetCelsius)
{
  bool heaterOn = (targetCelsius - HEATER_HYSTERESIS_CELSIUS) > currentCelsius;
  if (apiPostHeaterOn(heaterOn)) {
    // Actually turn on the relay
    // digitalWrite(HEATER_RELAY_PIN, heaterOn ? HIGH : LOW);
  }
}

void tempSetup() {
  dht.begin();
}

float tempRead() {
  float celsius = dht.readTemperature();
  return celsius;
}