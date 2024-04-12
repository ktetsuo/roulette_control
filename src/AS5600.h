#ifndef __AS5600_H__
#define __AS5600_H__

class TwoWire;

class AS5600
{
  TwoWire &_twoWire;
public:
  AS5600(TwoWire &twoWire);
  bool begin();
  unsigned int getRawAngle();
  static constexpr unsigned int maxRowAngle();
};

#endif // __AS5600_H__
