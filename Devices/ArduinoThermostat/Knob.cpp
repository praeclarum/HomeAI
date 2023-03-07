#include "Knob.h"
#include "Config.h"
#include "State.h"
#include "Api.h"
#include <AiEsp32RotaryEncoder.h>

static StateChangedEvent stateChanged(KNOB_TASK_ID);

#define ROTARY_ENCODER_VCC_PIN -1 /* 27 put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */

//depending on your encoder - try 1,2 or 4 to get expected behaviour
// #define ROTARY_ENCODER_STEPS 1
// #define ROTARY_ENCODER_STEPS 2
#define ROTARY_ENCODER_STEPS 4

static AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

static void IRAM_ATTR readEncoderISR()
{
	rotaryEncoder.readEncoder_ISR();
}

static void knobSetup() {
  rotaryEncoder.begin();
	rotaryEncoder.setup(readEncoderISR);
	//set boundaries and if values should cycle or not
	bool circleValues = false;
	rotaryEncoder.setBoundaries(50, 90, circleValues); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)

	rotaryEncoder.disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it
	rotaryEncoder.setAcceleration(0); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration
}

static int knobReadFahrenheit() {
    return (int)min(90l, max(50l, rotaryEncoder.readEncoder()));
}

static void knobUpdateFahrenheit(float fahrenheit) {
    rotaryEncoder.setEncoderValue(long(fahrenheit + 0.5f));
}

static bool ignoreNextChange = false;

static void knobLoop() {
  if (rotaryEncoder.encoderChanged()) {
    if (ignoreNextChange) {
      ignoreNextChange = false;
    }
    else {
      const auto targetFahrenheit = knobReadFahrenheit();
      // Serial.print("KNOB ");
      // Serial.print(targetFahrenheit);
      // Serial.println("F");
      auto now = millis();
      updateState(KNOB_TASK_ID, [targetFahrenheit, now](State &x) {
        x.knobChanging = true;
        x.knobSetFahrenheit = targetFahrenheit;
        x.knobChangeMillis = now;
      });
    }
  }
  const auto state = readState();
  if (state.knobChanging && (millis() - state.knobChangeMillis) > 3 * 1000) {
    float manuallySetTempC = f2c(state.knobSetFahrenheit);
    updateState(KNOB_TASK_ID, [manuallySetTempC](State &x) {
      x.knobChanging = false;
      x.targetCelsius = manuallySetTempC;
    });
    Serial.print("Committing manual setpoint: ");
    Serial.print(manuallySetTempC);
    Serial.println("C");
    vTaskDelay(1000 / portTICK_RATE_MS);
    if (apiPostManualTemperature(manuallySetTempC)) {
      vTaskDelay(1000 / portTICK_RATE_MS);
      if (apiPostTargetTemperature(manuallySetTempC)) {
      }
    }
  }
  if (stateChanged.wait(10)) {
    // Serial.println("KNOB SAW STATE CHANGE");
    const auto newState = readState();
    if (!newState.knobChanging) {
      ignoreNextChange = true;
      knobUpdateFahrenheit(c2f(newState.targetCelsius));
    }
  }
}

static void knobTask(void *arg) {
  knobSetup();
  for (;;) {
    knobLoop();
  }  
}

void knobStart() {
  subscribeToStateChanges(&stateChanged);
  xTaskCreatePinnedToCore(
    knobTask
    ,  "Knob"
    ,  8*1024  // Stack size
    ,  nullptr // Arg
    ,  TASK_PRIORITY  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);
}
