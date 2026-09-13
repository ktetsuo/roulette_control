#include <Arduino.h>
#include <M5Unified.h>
#include "RouletteDisplay.h"

static RouletteDisplay s_display(M5.Display);
static volatile bool s_buttonAPressed = false;

static constexpr int positionCountPerRevolution = 4096;
static constexpr int maximumSpeed = 136; // 136カウント/周期で約2回転/秒
static constexpr int acceleration = 8;
static constexpr int deceleration = 1;

void intarvalTaskStep()
{
	static int currentPos = 0;
	static int currentSpeed = 0;

	if (s_buttonAPressed)
	{
		currentSpeed += acceleration;
		if (currentSpeed > maximumSpeed)
		{
			currentSpeed = maximumSpeed;
		}
	}
	else
	{
		currentSpeed -= deceleration;
		if (currentSpeed < 0)
		{
			currentSpeed = 0;
		}
	}

	currentPos = (currentPos + currentSpeed) % positionCountPerRevolution;
	s_display.setCurrentPos(currentPos);
	s_display.setCurrentSpeed(currentSpeed);
}

void intervalTask(void *)
{
	constexpr TickType_t taskPeriod = pdMS_TO_TICKS(16);
	TickType_t lastWakeTime = xTaskGetTickCount();
	for (;;)
	{
		intarvalTaskStep();
		vTaskDelayUntil(&lastWakeTime, taskPeriod); // 約60Hzの周期を維持
	}
}

void setup()
{
	auto cfg = M5.config();
	cfg.serial_baudrate = 115200;
	M5.begin(cfg);
	s_display.init();
	// 周期タスクを作成
	xTaskCreate(intervalTask, "DisplayTask", 4096, nullptr, 1, nullptr);
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
	// Aボタン
	s_buttonAPressed = M5.BtnA.isPressed();
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
