#include "State.h"
#include "Api.h"
#include "Config.h"
#include "Thermometer.h"
#include "Knob.h"
#include "Display.h"
#include "Heater.h"

static unsigned long lastReadMillis = 0;
static float lastCelsius = 0;

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();

  stateSetup();

  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  heaterStart();

  displayStart();
  thermometerStart();
  knobStart();
  apiSetup();
}

void loop() {
  const auto nowMillis = millis();

  const bool shouldRead = lastReadMillis == 0 || ((nowMillis - lastReadMillis) > 5 * 60 * 1000);

  const State state = readState();
  lastCelsius = state.thermometerCelsius;

  if (lastCelsius > 1 && shouldRead) {
    Serial.print("READ ");
    Serial.print(lastCelsius);
    Serial.print("C ");
    Serial.print(lastCelsius * 9.0f / 5.0f + 32.0f);
    Serial.println("F");

    bool readSucceeded = false;
    if (apiPostTemperature(lastCelsius)) {
      vTaskDelay(1000 / portTICK_RATE_MS);
      float targetCelsius = 0.0;
      if (apiGetTargetTemperature(&targetCelsius)) {
        updateState(SERVER_TASK_ID, [targetCelsius](State &x) {
          x.targetCelsius = targetCelsius;
        });
        Serial.print("TARGET ");
        Serial.print(targetCelsius);
        Serial.println("C ");
        vTaskDelay(1000 / portTICK_RATE_MS);
        if (apiPostTargetTemperature(targetCelsius)) {
          readSucceeded = true;
        }
      }
    }

    if (readSucceeded) {
      lastReadMillis = millis();
    }
    else {
      updateState(SERVER_TASK_ID, [](State &x) {
        x.networkError = true;
      });
      Serial.println("NETWORK ERROR, RESTARTING AFTER A MINUTE...");
      digitalWrite(HEATER_PIN, LOW);
      // displayError();
      delay(60000);
      ESP.restart();
    }
  }

  vTaskDelay(1000 / portTICK_RATE_MS);
}
