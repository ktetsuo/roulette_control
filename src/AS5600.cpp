#include "AS5600.h"
#include <Arduino.h>
#include <Wire.h>

static constexpr uint8_t AS5600_AS5601_DEV_ADDRESS = 0x36;
static constexpr uint8_t AS5600_AS5601_REG_RAW_ANGLE = 0x0C;

AS5600::AS5600(TwoWire &twoWire)
    : _twoWire(twoWire)
{
}
bool AS5600::begin()
{
  // Wire.begin()は事前に実行しておいてください
  return true;
}
unsigned int AS5600::getRawAngle()
{
  _twoWire.beginTransmission(AS5600_AS5601_DEV_ADDRESS);
  _twoWire.write(AS5600_AS5601_REG_RAW_ANGLE);
  _twoWire.endTransmission(false);
  _twoWire.requestFrom(AS5600_AS5601_DEV_ADDRESS, 2);
  unsigned int rawAngle = 0;
  rawAngle = ((uint16_t)_twoWire.read() << 8) & 0x0F00;
  rawAngle |= (uint16_t)_twoWire.read();
  return rawAngle;
}
