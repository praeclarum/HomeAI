#include "State.h"
#include "Server.h"
#include "Config.h"
#include "Thermometer.h"
#include "Knob.h"
#include "Display.h"
#include "Heater.h"

void setup() {

  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();

  stateSetup();

  serverStart();

  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  heaterStart();

  displayStart();
  thermometerStart();
  knobStart();
}

void loop() {
  vTaskDelay(1000 / portTICK_RATE_MS);
}
