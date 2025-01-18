#ifndef __BOOTSELIN_H__
#define __BOOTSELIN_H__

#include "DigitalIn.h"

class BootSelIn : public IDigitalIn
{
  virtual bool isOn() override;
};

#endif // __BOOTSELIN_H__
