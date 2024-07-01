#ifndef __ENCODERSPEED_H__
#define __ENCODERSPEED_H__

template <typename PosType, typename SpeedType>
class EncoderSpeed
{
  PosType _lastPos;
  const PosType _maxPos;
  SpeedType _speed;

public:
  EncoderSpeed(PosType maxPos);
  SpeedType update(PosType currentPos);
  SpeedType speed() const;
};
#include "EncoderSpeed.h"

template <typename PosType, typename SpeedType>
EncoderSpeed<PosType, SpeedType>::EncoderSpeed(PosType maxPos)
    : _lastPos(0), _maxPos(maxPos) {}

template <typename PosType, typename SpeedType>
SpeedType EncoderSpeed<PosType, SpeedType>::update(PosType currentPos)
{
  SpeedType posDiff = static_cast<SpeedType>(currentPos) - static_cast<SpeedType>(_lastPos);

  // 符号が変わっている場合は、エンコーダが巻き戻った可能性がある
  if (posDiff > static_cast<SpeedType>(_maxPos) / 2)
  {
    posDiff -= _maxPos;
  }
  else if (posDiff < -static_cast<SpeedType>(_maxPos) / 2)
  {
    posDiff += _maxPos;
  }

  _lastPos = currentPos;
  _speed = posDiff;
  return _speed;
}

template <typename PosType, typename SpeedType>
SpeedType EncoderSpeed<PosType, SpeedType>::speed() const
{
  return _speed;
}

#endif // __ENCODERSPEED_H__
