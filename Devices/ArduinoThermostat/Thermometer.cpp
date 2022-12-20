#include "Thermometer.h"
#include "Config.h"
#include <DHT.h>

#define DHT_TYPE DHT22

static DHT dht(DHT_PIN, DHT_TYPE);

void thermometerSetup() {
  dht.begin();
}

float thermometerReadCelsius() {
  float celsius = dht.readTemperature();
  return celsius;
}
