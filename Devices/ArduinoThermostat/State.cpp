#include "State.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static State sharedState = { 0 };

static SemaphoreHandle_t sharedStateMutex = 0;

void stateSetup() {
  sharedStateMutex = xSemaphoreCreateMutex();
}

State readState() {
  unsigned long waitMillis = 100l;
  xSemaphoreTake(sharedStateMutex, waitMillis / portTICK_PERIOD_MS);
  State copiedState = sharedState;
  xSemaphoreGive(sharedStateMutex);
  return copiedState;
}

void updateState(std::function<void(State&)> updater) {
  unsigned long waitMillis = 100l;
  xSemaphoreTake(sharedStateMutex, waitMillis / portTICK_PERIOD_MS);
  updater(sharedState);
  xSemaphoreGive(sharedStateMutex);
}
