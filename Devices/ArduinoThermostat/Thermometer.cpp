#include "Thermometer.h"
#include "Config.h"
#include <DHT.h>

#define DHT_TYPE DHT22

static DHT dht(DHT_PIN, DHT_TYPE);

void thermometerSetup() {
  dht.begin();
}

bool thermometerReadCelsius(float &celsius) {
  celsius = dht.readTemperature();
  return !isnan(celsius);
}
