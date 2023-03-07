#pragma once

void serverStart();

bool serverPostTemperatureAsync(float celsius);
bool serverPostManualTemperatureAsync(float celsius);
bool serverPostTargetTemperatureAsync(float celsius);
bool serverPostHeaterOnAsync(bool on);
