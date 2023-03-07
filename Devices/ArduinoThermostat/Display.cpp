#include <Arduino.h>
#include "Display.h"
#include "State.h"
#include "Config.h"
#include <TM1637Display.h>

static StateChangedEvent stateChanged(DISPLAY_TASK_ID);

static TM1637Display tm(DISPLAY_CLK_PIN, DISPLAY_DIO_PIN);

void displaySetup() {
  // Serial.println("displaySetup");
  tm.setBrightness(2);
  tm.showNumberDec(0);
}

void displayUpdate()//float currentC, float targetC, bool showTarget)
{
  // Serial.println("displayUpdate");
  const State state = readState();

  const auto currentC = state.thermometerCelsius;
  const auto targetC = state.targetCelsius;
  auto targetF = int(c2f(targetC) + 0.5f);
  auto showTarget = state.isHeaterOn;
  if (state.knobChanging) {
    targetF = state.knobSetFahrenheit;
    showTarget = true;
  }
  
  // displayUpdate(lastCelsius, manuallySetTemp ? manuallySetTempC : lastTargetCelsius, manuallySetTemp || isHeaterOn);

  const auto currentF = int(c2f(currentC) + 0.5f);
  const uint8_t dots = showTarget ? 0b01000000 : 0;

  tm.showNumberDecEx(currentF, dots, true, 2, 2);
  if (showTarget) {
    tm.showNumberDecEx(targetF, dots, true, 2, 0);
  }
  else {
    const uint8_t segments[2] = { 0, 0 };
    tm.setSegments(segments, 2, 0);
  } 
}

void displayError()
{
  tm.showNumberHexEx(0xE);  
}

static void displayLoop() {
  if (stateChanged.wait(10)) {
    // Serial.println("DISPLAY SAW STATE CHANGE");
    displayUpdate();
  }
}


static void displayTask(void *arg) {
  displaySetup();
  for (;;) {
    displayLoop();
  }  
}

void displayStart() {
  subscribeToStateChanges(&stateChanged);
  xTaskCreatePinnedToCore(
    displayTask
    ,  "Display"
    ,  4*1024  // Stack size
    ,  nullptr // Arg
    ,  TASK_PRIORITY  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);
}
