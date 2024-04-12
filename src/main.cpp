#include <Arduino.h>
#include <Wire.h>
#include "Timer.h"
#include "MaxMinHolder.h"
#include "DigitalOut.h"
#include "BootSelIn.h"
#include "ChatteringFilteredDigitalInWatcher.h"
#include "AS5600.h"
#include "ValueChangeWatcher.h"
#include "IntervalWatcher.h"
#include "HeartBeatLED.h"

static constexpr pin_size_t BRAKE_CW_PIN = 2;
static constexpr pin_size_t BRAKE_CCW_PIN = 3;

static BootSelIn bootSelIn;
static ChatteringFilteredDigitalInWatcher bootSelWatcher(bootSelIn, 50, false);

static AS5600 as5600(Wire);
static ValueChangeWatcher<unsigned int> encoderChangeWatcher;

static DigitalOut boardLED(LED_BUILTIN);
static HeartBeatLED heartBeatLED(boardLED, 1000);

void setup()
{
  pinMode(BRAKE_CW_PIN, OUTPUT);
  pinMode(BRAKE_CCW_PIN, OUTPUT);
  digitalWrite(BRAKE_CW_PIN, LOW);
  digitalWrite(BRAKE_CCW_PIN, LOW);
  heartBeatLED.setup();
  Serial.begin(115200);
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();
  Wire.setClock(1000000);
  as5600.begin();
}

void loop()
{
  static IntervalWatcher intervalWatcher("loop", Serial);
  intervalWatcher.update();
  heartBeatLED.loop();
  bootSelWatcher.update();
  if (bootSelWatcher.isRisingEdge())
  {
    Serial.println("BOOTSEL ON!");
  }
  else if (bootSelWatcher.isFallingEdge())
  {
    Serial.println("BOOTSEL OFF!");
  }
  encoderChangeWatcher.update(as5600.getRawAngle());
  if(encoderChangeWatcher.isChanged()){
    Serial.print("Angle: ");
    Serial.println(encoderChangeWatcher.value());
  }

  if (Serial.available() > 0)
  {
    const int c = Serial.read();
    switch (c)
    {
    case '0':
      digitalWrite(BRAKE_CW_PIN, LOW);
      digitalWrite(BRAKE_CCW_PIN, LOW);
      Serial.println("CW: OFF, CCW:OFF");
      intervalWatcher.reset();
      break;
    case '1':
      digitalWrite(BRAKE_CW_PIN, HIGH);
      Serial.println("CW: ON");
      break;
    case '2':
      digitalWrite(BRAKE_CW_PIN, LOW);
      Serial.println("CW: OFF");
      break;
    case '3':
      digitalWrite(BRAKE_CCW_PIN, HIGH);
      Serial.println("CCW: ON");
      break;
    case '4':
      digitalWrite(BRAKE_CCW_PIN, LOW);
      Serial.println("CCW: OFF");
      break;
    case 'r':
    {
      Serial.println(as5600.getRawAngle());
    }
    break;
    default:
      break;
    }
  }
}
