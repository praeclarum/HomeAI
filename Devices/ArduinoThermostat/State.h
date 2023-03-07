#pragma once

#include <functional>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

struct State {
  float targetCelsius;

  float thermometerCelsius;

  bool isHeaterOn;

  bool networkError;

  bool knobChanging;
  unsigned long knobChangeMillis;
  int knobSetFahrenheit;
};

class StateChangedEvent {
  int taskId;
  QueueHandle_t queue;
public:
  StateChangedEvent(int taskId) : taskId(taskId), queue(xQueueCreate(4, sizeof(int))) {
  }
  bool wait(unsigned long millis) {
    int message = 0;
    auto r = xQueueReceive(queue, &message, millis / portTICK_RATE_MS);
    return r == pdTRUE;
  }
  void trigger(int senderTaskId) {
    if (senderTaskId == taskId)
      return;
    unsigned long millis = 10;
    int message = senderTaskId;
    auto r = xQueueSend(queue, &message, millis / portTICK_RATE_MS);
  }
};

void stateSetup();
State readState();
void subscribeToStateChanges(StateChangedEvent *event);
void updateState(int taskId, std::function<void(State&)> updater);

