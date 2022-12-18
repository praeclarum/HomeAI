#include "Api.h"

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

  // apiSetup();
}

void loop() {
  
  // apiLoop();

  float currentCelsius = tempRead();
  // apiPostTemperature(currentCelsius);
  Serial.print("READ ");
  Serial.print(currentCelsius);
  Serial.print("C ");
  Serial.print(currentCelsius * 9.0f / 5.0f + 32.0f);
  Serial.println("F");
  delay(1000);  
}

void tempSetup() {
  dht.begin();
}

float tempRead() {
  float celsius = dht.readTemperature();
  return celsius;
}