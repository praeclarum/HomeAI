#pragma once

void serverStart();

bool serverPostTemperature(float celsius);
bool serverPostManualTemperature(float celsius);
bool serverPostTargetTemperature(float celsius);
bool serverPostHeaterOn(bool on);
