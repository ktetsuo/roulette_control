#ifndef __PWM_H__
#define __PWM_H__

#include <Arduino.h>
#include "Duty.h"

class PWM
{
  const pin_size_t _pin;
  const unsigned int _maxValue;
  const unsigned int _freq;
  UnsignedDuty<unsigned int> _duty;

public:
  explicit PWM(pin_size_t pin, unsigned int maxValue, unsigned int freq);
  void setup();
  void setPercent(unsigned int percent);
};

#endif // __PWM_H__
