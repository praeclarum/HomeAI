#include <functional>
#pragma once

struct State {
  bool knobChanging;
  unsigned long knobChangeMillis;
  int knobSetFahrenheit;
};

void stateSetup();
State readState();
void updateState(std::function<void(State&)> updater);

