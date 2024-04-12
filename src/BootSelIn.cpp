#include "BootSelIn.h"
#include <Arduino.h>

bool BootSelIn::isOn()
{
  return BOOTSEL;
}
