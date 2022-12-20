#include "Knob.h"
#include "Config.h"
#include <AiEsp32RotaryEncoder.h>

#define ROTARY_ENCODER_VCC_PIN -1 /* 27 put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */

//depending on your encoder - try 1,2 or 4 to get expected behaviour
// #define ROTARY_ENCODER_STEPS 1
// #define ROTARY_ENCODER_STEPS 2
#define ROTARY_ENCODER_STEPS 4

static AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

void IRAM_ATTR readEncoderISR()
{
	rotaryEncoder.readEncoder_ISR();
}

void knobSetup() {
  rotaryEncoder.begin();
	rotaryEncoder.setup(readEncoderISR);
	//set boundaries and if values should cycle or not
	bool circleValues = false;
	rotaryEncoder.setBoundaries(50, 90, circleValues); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)

	rotaryEncoder.disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it
	rotaryEncoder.setAcceleration(0); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration
}

bool knobChanged() {
    return rotaryEncoder.encoderChanged();
}

long knobReadFahrenheit() {
    return rotaryEncoder.readEncoder();
}

void knobSetFahrenheit(float fahrenheit) {
    rotaryEncoder.setEncoderValue(long(fahrenheit + 0.5f));
}
