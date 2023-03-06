#include "Knob.h"
#include "Config.h"
#include "State.h"
#include <AiEsp32RotaryEncoder.h>


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

static long knobReadFahrenheit() {
    return rotaryEncoder.readEncoder();
}

static void knobSetFahrenheit(float fahrenheit) {
    rotaryEncoder.setEncoderValue(long(fahrenheit + 0.5f));
}

static void knobLoop() {
  if (rotaryEncoder.encoderChanged()) {
    const auto targetFahrenheit = knobReadFahrenheit();
    Serial.print("KNOB ");
    Serial.print(targetFahrenheit);
    Serial.println("F");
    auto now = millis();
    updateState([targetFahrenheit, now](State &x) {
      x.knobChanging = true;
      x.knobSetFahrenheit = (int)(min(90l, max(32l, targetFahrenheit)));
      x.knobChangeMillis = now;
    });
  }
  vTaskDelay(10 / portTICK_PERIOD_MS);
}

static void knobTask(void *arg) {
  knobSetup();
  for (;;) {
    knobLoop();
  }  
}

void knobStart() {
  xTaskCreatePinnedToCore(
    knobTask
    ,  "Knob"
    ,  1024  // Stack size
    ,  nullptr // Arg
    ,  TASK_PRIORITY  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);
}
