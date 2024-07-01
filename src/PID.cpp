#include "PID.h"
#include <algorithm>

PID::PID(float kP, float kI, float kD, float dt, float outputMin, float outputMax, float targetValue)
    : _kP(kP), _kI(kI), _kD(kD), _dt(dt),
      _outputMin(outputMin), _outputMax(outputMax),
      _targetValue(targetValue),
      _prevError(0.0f), _prevProp(0), _output(0.0f)
{
}

float PID::kP() const
{
  return _kP;
}
void PID::kP(float k)
{
  _kP = k;
}

float PID::kI() const
{
  return _kI;
}
void PID::kI(float k)
{
  _kI = k;
}

float PID::kD() const
{
  return _kD;
}
void PID::kD(float k)
{
  _kD = k;
}

void PID::targetValue(float target)
{
  _targetValue = target;
}

float PID::targetValue() const
{
  return _targetValue;
}

float PID::output() const
{
  return _output;
}

void PID::update(float currentValue)
{
  const float error = _targetValue - currentValue;
  const float prop = (error - _prevError) / _dt;
  const float deriv = (prop - _prevProp) / _dt;
  _prevError = error;
  _prevProp = prop;

  const float output = _output + _kP * prop + _kI * error + _kD * deriv;
  //_output = std::clamp(output, _outputMin, _outputMax);
  _output = output;
}
