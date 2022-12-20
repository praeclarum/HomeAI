#pragma once

void apiSetup();

bool apiPostTemperature(float celsius);
bool apiPostManualTemperature(float celsius);
bool apiPostTargetTemperature(float celsius);
bool apiPostHeaterOn(bool on);
bool apiGetTargetTemperature(float *targetCelsius);
