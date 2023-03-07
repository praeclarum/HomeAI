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

#define MAX_SUBSCRIBERS 16
struct Subscribers {
  size_t numSubscribers;
  StateChangedEvent *subscribers[MAX_SUBSCRIBERS];
  Subscribers() : numSubscribers(0) {}
};
static Subscribers subscribers;

void subscribeToStateChanges(StateChangedEvent *event) {
  unsigned long waitMillis = 100l;
  xSemaphoreTake(sharedStateMutex, waitMillis / portTICK_PERIOD_MS);
  if (subscribers.numSubscribers < MAX_SUBSCRIBERS) {
    subscribers.subscribers[subscribers.numSubscribers] = event;
    subscribers.numSubscribers++;
  }
  xSemaphoreGive(sharedStateMutex);
}

void updateState(int taskId, std::function<void(State&)> updater) {
  unsigned long waitMillis = 100l;
  xSemaphoreTake(sharedStateMutex, waitMillis / portTICK_PERIOD_MS);
  updater(sharedState);
  Subscribers subs = subscribers;
  xSemaphoreGive(sharedStateMutex);
  for (size_t i = 0; i < subs.numSubscribers; i++) {
    subs.subscribers[i]->trigger(taskId);
  }
}
