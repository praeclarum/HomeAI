#include <Arduino.h>
#include "Heater.h"
#include "Config.h"
#include "State.h"
#include "Server.h"

static StateChangedEvent stateChanged(HEATER_TASK_ID);

static void heaterSetup() {
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
}

static void control() {
  // Serial.println("HEATER CONTROL");
  const State state = readState();
  const float currentCelsius = state.thermometerCelsius;
  const float targetCelsius = state.targetCelsius;

  bool heaterShouldBeOn = false;

  if (currentCelsius > 1.0f) {
    if (state.isHeaterOn) {
      // Turn off if current temp greater than setpoint plus hysteresis
      const auto heaterShouldBeOff = currentCelsius > targetCelsius + HEATER_HYSTERESIS_CELSIUS;
      heaterShouldBeOn = !heaterShouldBeOff;
    }
    else {
      // Turn on if current temp is less then setpoint minus hysteresis
      heaterShouldBeOn = currentCelsius < targetCelsius - HEATER_HYSTERESIS_CELSIUS;
    }
  }
  auto isHeaterOn = heaterShouldBeOn;
  digitalWrite(HEATER_PIN, isHeaterOn ? HIGH : LOW);
  if (state.isHeaterOn != isHeaterOn) {
    updateState(HEATER_TASK_ID, [isHeaterOn](State &x) {
      x.isHeaterOn = isHeaterOn;
    });
    if (isHeaterOn) {
      Serial.println("HEATER ON");
    }
    else {
      Serial.println("HEATER OFF");
    }
    serverPostHeaterOnAsync(isHeaterOn);
  }
}

static void heaterLoop() {
  control();
  if (stateChanged.wait(7000)) {
    //Serial.println("HEATER SAW STATE CHANGE");
  }
}

static void heaterTask(void *arg) {
  heaterSetup();
  for (;;) {
    heaterLoop();
  }  
}

void heaterStart() {
  subscribeToStateChanges(&stateChanged);
  xTaskCreatePinnedToCore(
    heaterTask
    ,  "Heater"
    ,  8*1024  // Stack size
    ,  nullptr // Arg
    ,  TASK_PRIORITY  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);
}
