#include "PWM.h"

PWM::PWM(pin_size_t pin, unsigned int maxValue, unsigned int freq)
    : _pin(pin), _maxValue(maxValue), _duty(maxValue, 0), _freq(freq)
{
}

void PWM::setup()
{
  analogWriteFreq(_freq);
  analogWriteRange(_maxValue);
  analogWrite(_pin, _duty.currentValue());
}

void PWM::setPercent(unsigned int percent)
{
  _duty.setPercent(static_cast<float>(percent));
  analogWrite(_pin, _duty.currentValue());
}
