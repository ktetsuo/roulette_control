#include <Arduino.h>
#include <M5Unified.h>
#include "RouletteDisplay.h"

static RouletteDisplay s_display(M5.Display);

void setup()
{
	auto cfg = M5.config();
	M5.begin(cfg);
	s_display.init();
}

void loop()
{
	M5.update();
	s_display.update();
}
