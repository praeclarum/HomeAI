#include "freertos/FreeRTOS.h"
#include "Thermometer.h"
#include "Config.h"
#include "State.h"
#include <DHT.h>

float f2c(float f) { return (f - 32.0f) * 5.0f / 9.0f; }
float c2f(float c) { return c * 9.0f / 5.0f + 32.0f; }

#define DHT_TYPE DHT22

static DHT dht(DHT_PIN, DHT_TYPE);

static void thermometerSetup() {
  // Serial.println("THERMOMETER Setup");
  dht.begin();
}

static bool thermometerReadCelsius(float &celsius) {
  celsius = dht.readTemperature();
  return !isnan(celsius);
}

static void thermometerLoop() {
  const int readingsToAverage = 5;
  float sumC = 0.0f;
  int numReadings = 0;
  while (numReadings < readingsToAverage) {
    vTaskDelay(1000 / portTICK_RATE_MS);
    // Serial.println("THERMO Read");
    float c = 0;
    if (thermometerReadCelsius(c)) {
      sumC += c;
      numReadings++;
    }
  }
  float celsius = sumC / readingsToAverage;
  Serial.print("THERMOMETER READ ");
  Serial.print(celsius);
  Serial.println("C");
  updateState(THERMOMETER_TASK_ID, [celsius](State &x) {
    x.thermometerCelsius = celsius;
  });
}

static void thermometerTask(void *arg) {
  thermometerSetup();
  for (;;) {
    thermometerLoop();
  }  
}

void thermometerStart() {
  Serial.println("Starting THERMOMETER");
  xTaskCreatePinnedToCore(
    thermometerTask
    ,  "Thermometer"
    ,  4*1024  // Stack size
    ,  nullptr // Arg
    ,  TASK_PRIORITY  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);
}
