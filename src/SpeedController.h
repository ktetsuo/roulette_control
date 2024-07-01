#ifndef __SPEEDCONTROLLER_H__
#define __SPEEDCONTROLLER_H__

class SpeedController
{
  float _targetSpeed;

public:
  SpeedController();
  float currentSpeed() const;
  float targetSpeed() const;
  void targetSpeed(float targetSpeed);
};

#endif // __SPEEDCONTROLLER_H__
