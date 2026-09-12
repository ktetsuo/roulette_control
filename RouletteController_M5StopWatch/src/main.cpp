#include <Arduino.h>
#include <M5Unified.h>
#include "RouletteDisplay.h"

static RouletteDisplay s_display(M5.Display);

void setup()
{
	auto cfg = M5.config();
	cfg.serial_baudrate = 115200;
	M5.begin(cfg);
	s_display.init();
}

void loop()
{
	// ループ間隔を測定
	static unsigned long _t0 = micros();
	const unsigned long t = micros();
	const unsigned long dt = t - _t0;
	_t0 = t;
	Serial.print("Loop time (us): ");
	Serial.println(dt);
	// M5の更新
	M5.update();
	// Aボタンが押されている間回転する
	static int currentPos = 0;
	if (M5.BtnA.isPressed())
	{
		currentPos = (currentPos + 16) % 4096;
		s_display.setCurrentPos(currentPos);
		Serial.print("Current position: ");
		Serial.println(currentPos);
	}
	// Bボタンが押されたら数字をカウントアップする
	static int currentNumber = 1;
	if (M5.BtnB.wasPressed())
	{
		currentNumber++;
		currentNumber = (currentNumber - 1) % 10 + 1; // 1から10の範囲に制限
		s_display.setCurrentNumber(currentNumber);
		Serial.print("Current number: ");
		Serial.println(currentNumber);
	}
	// ディスプレイの更新
	s_display.update();
}
