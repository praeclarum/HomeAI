#pragma once

void apiStart();

bool apiPostTemperature(float celsius);
bool apiPostManualTemperature(float celsius);
bool apiPostTargetTemperature(float celsius);
bool apiPostHeaterOn(bool on);
