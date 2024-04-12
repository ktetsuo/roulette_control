#ifndef __TIMER_H__
#define __TIMER_H__

#include <Arduino.h>

////////////////////////////////////////////////////////////////////////////////
class MilliSecTimer
{
  unsigned long _t0;
public:
  MilliSecTimer();
  void reset();
  unsigned long elapsedTime() const;
};

////////////////////////////////////////////////////////////////////////////////
class MilliSecTimeoutTimer
{
  unsigned long _timeout;
  MilliSecTimer _timer;

public:
  explicit MilliSecTimeoutTimer(unsigned long timeout);
  void reset();
  bool isTimeout() const;
};

////////////////////////////////////////////////////////////////////////////////
class MilliSecIntervalTimer
{
  MilliSecTimeoutTimer _timeoutTimer;
  void (*_handler)(void *);
  void * _param;
public:
  explicit MilliSecIntervalTimer(unsigned long timeout, void (*handler)(void *), void* param);
  void reset();
  void update();
};

////////////////////////////////////////////////////////////////////////////////
class MicroSecTimer
{
  unsigned long _t0;
public:
  MicroSecTimer();
  void reset();
  unsigned long elapsedTime() const;
};

////////////////////////////////////////////////////////////////////////////////
class MicroSecTimeoutTimer
{
  unsigned long _timeout;
  MicroSecTimer _timer;

public:
  explicit MicroSecTimeoutTimer(unsigned long timeout);
  void reset();
  bool isTimeout() const;
};

////////////////////////////////////////////////////////////////////////////////
class MicroSecIntervalTimer
{
  MicroSecTimeoutTimer _timeoutTimer;
  void (*_handler)(void *);
  void * _param;
public:
  explicit MicroSecIntervalTimer(unsigned long timeout, void (*handler)(void *), void* param);
  void reset();
  void update();
};

#endif // __TIMER_H__
