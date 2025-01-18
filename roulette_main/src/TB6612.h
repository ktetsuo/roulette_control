#ifndef __TB6612_H__
#define __TB6612_H__

#include "DigitalOut.h"
#include "PWM.h"
#include <type_traits>

class TB6612
{
  IDigitalOut &_in1;
  IDigitalOut &_in2;
  PWM &_pwm;

public:
  TB6612(IDigitalOut &in1, IDigitalOut &in2, PWM &_pwm);
  void setup();
  void brake();
  void free();
  template <typename PercentType>
  void drive(PercentType percent);
};
TB6612::TB6612(IDigitalOut &in1, IDigitalOut &in2, PWM &pwm)
    : _in1(in1), _in2(in2), _pwm(pwm)
{
}

void TB6612::setup()
{
}

void TB6612::brake()
{
  _in1.turnOn();
  _in2.turnOn();
  _pwm.setPercent(0);
}

void TB6612::free()
{
  _in1.turnOff();
  _in2.turnOff();
  _pwm.setPercent(0);
}
template <typename PercentType>
void TB6612::drive(PercentType percent)
{
  static_assert(std::is_signed<PercentType>());
  const PercentType outputPercent = abs(percent);
  if (percent > 0)
  {
    _in1.turnOn();
    _in2.turnOff();
  }
  else
  {
    _in1.turnOff();
    _in2.turnOn();
  }
  _pwm.setPercent(outputPercent);
}

#endif // __TB6612_H__
