#pragma once

// PINS

#define DISPLAY_CLK_PIN 32
#define DISPLAY_DIO_PIN 25

#define DHT_PIN 33

#define ROTARY_ENCODER_A_PIN 34
#define ROTARY_ENCODER_B_PIN 35
#define ROTARY_ENCODER_BUTTON_PIN 26

#define HEATER_PIN 5

// TASKS

#define THERMOMETER_TASK_ID 1
#define DISPLAY_TASK_ID     2
#define CONTROL_TASK_ID     3
#define KNOB_TASK_ID        4

#define TASK_PRIORITY 2

// Common Functions

float f2c(float f);
float c2f(float c);
