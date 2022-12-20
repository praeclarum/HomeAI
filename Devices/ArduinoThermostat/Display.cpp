#include "Display.h"
#include "Config.h"
#include <TM1637Display.h>

static TM1637Display tm(DISPLAY_CLK_PIN, DISPLAY_DIO_PIN);

void displaySetup() {
  tm.setBrightness(2);
  tm.showNumberDec(0);
}

void displayUpdate(float currentC, float targetC, bool heaterOn)
{
  const auto currentF = int(currentC*9.0f/5.0f + 32.0f + 0.5f);
  const auto targetF = int(targetC*9.0f/5.0f + 32.0f + 0.5f);
  const bool showTarget = heaterOn;
  const uint8_t dots = showTarget ? 0b01000000 : 0;

  tm.showNumberDecEx(currentF, dots, true, 2, 2);
  if (showTarget) {
    tm.showNumberDecEx(targetF, dots, true, 2, 0);
  }
  else {
    const uint8_t segments[2] = { 0, 0 };
    tm.setSegments(segments, 2, 0);
  } 
}


