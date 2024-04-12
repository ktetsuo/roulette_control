#include "Timer.h"

////////////////////////////////////////////////////////////////////////////////
MilliSecTimer::MilliSecTimer()
    : _t0(0)
{
}
void MilliSecTimer::reset()
{
  _t0 = millis();
}
unsigned long MilliSecTimer::elapsedTime() const
{
  return millis() - _t0;
}

////////////////////////////////////////////////////////////////////////////////
MilliSecTimeoutTimer::MilliSecTimeoutTimer(unsigned long timeout)
    : _timeout(timeout)
{
}
void MilliSecTimeoutTimer::reset()
{
  _timer.reset();
}
bool MilliSecTimeoutTimer::isTimeout() const
{
  return _timeout <= _timer.elapsedTime();
}

////////////////////////////////////////////////////////////////////////////////
MilliSecIntervalTimer::MilliSecIntervalTimer(unsigned long timeout, void (*handler)(void *), void *param)
    : _timeoutTimer(timeout), _handler(handler), _param(param)
{
}
void MilliSecIntervalTimer::reset()
{
}
void MilliSecIntervalTimer::update()
{
  if (_timeoutTimer.isTimeout())
  {
    _timeoutTimer.reset();
    if (_handler)
    {
      _handler(_param);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
MicroSecTimer::MicroSecTimer()
    : _t0(0)
{
}
void MicroSecTimer::reset()
{
  _t0 = micros();
}
unsigned long MicroSecTimer::elapsedTime() const
{
  return micros() - _t0;
}

////////////////////////////////////////////////////////////////////////////////
MicroSecTimeoutTimer::MicroSecTimeoutTimer(unsigned long timeout)
    : _timeout(timeout)
{
}
void MicroSecTimeoutTimer::reset()
{
  _timer.reset();
}
bool MicroSecTimeoutTimer::isTimeout() const
{
  return _timeout <= _timer.elapsedTime();
}

////////////////////////////////////////////////////////////////////////////////
MicroSecIntervalTimer::MicroSecIntervalTimer(unsigned long timeout, void (*handler)(void *), void *param)
    : _timeoutTimer(timeout), _handler(handler), _param(param)
{
}
void MicroSecIntervalTimer::reset()
{
}
void MicroSecIntervalTimer::update()
{
  if (_timeoutTimer.isTimeout())
  {
    _timeoutTimer.reset();
    if (_handler)
    {
      _handler(_param);
    }
  }
}
