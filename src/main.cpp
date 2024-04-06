#include <Arduino.h>

static constexpr pin_size_t BRAKE_CW_PIN = 2;
static constexpr pin_size_t BRAKE_CCW_PIN = 3;
static constexpr pin_size_t BOARD_LED_PIN = LED_BUILTIN;

class HeartBeatLED
{
  const pin_size_t _pin;
  const unsigned long _intervalms;
  unsigned long _lastms;
  bool _on;

public:
  explicit HeartBeatLED(pin_size_t pin, unsigned long intervalms);
  void setup();
  void loop();
};

HeartBeatLED::HeartBeatLED(pin_size_t pin, unsigned long intervalms)
    : _pin(pin), _intervalms(intervalms), _lastms(0), _on(false)
{
}
void HeartBeatLED::setup()
{
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);
  _lastms = millis();
  _on = false;
}
void HeartBeatLED::loop()
{
  const unsigned long ms = millis();
  if (ms - _lastms >= _intervalms)
  {
    _lastms = ms;
    if (_on)
    {
      _on = false;
      digitalWrite(_pin, LOW);
    }
    else
    {
      _on = true;
      digitalWrite(_pin, HIGH);
    }
  }
}

static HeartBeatLED heartBeatLED(BOARD_LED_PIN, 1000);

void setup()
{
  pinMode(BRAKE_CW_PIN, OUTPUT);
  pinMode(BRAKE_CCW_PIN, OUTPUT);
  digitalWrite(BRAKE_CW_PIN, LOW);
  digitalWrite(BRAKE_CCW_PIN, LOW);
  heartBeatLED.setup();
  Serial.begin(115200);
}

void loop()
{
  heartBeatLED.loop();
  if (Serial.available() > 0)
  {
    const int c = Serial.read();
    switch (c)
    {
    case '0':
      digitalWrite(BRAKE_CW_PIN, LOW);
      digitalWrite(BRAKE_CCW_PIN, LOW);
      Serial.println("CW: OFF, CCW:OFF");
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
    default:
      break;
    }
  }
}
