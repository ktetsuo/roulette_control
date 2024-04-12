#ifndef __DIGITALIN_H__
#define __DIGITALIN_H__

#include <Arduino.h>

class IDigitalIn
{
public:
  virtual bool isOn() = 0;
};

////////////////////////////////////////////////////////////////////////////////
class DigitalIn : public IDigitalIn
{
  const pin_size_t _pin;
  const int _pinMode;

public:
  DigitalIn(pin_size_t pin, int pinMode);
  void setup();
  virtual bool isOn() override;
};

#endif // __DIGITALIN_H__
