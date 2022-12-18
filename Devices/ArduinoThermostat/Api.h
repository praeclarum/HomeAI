#pragma once

void apiSetup();

bool apiPostTemperature(float celsius);
bool apiPostTargetTemperature(float celsius);
bool apiGetTargetTemperature(float *targetCelsius);
