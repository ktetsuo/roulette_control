#ifndef __MAXMINHOLDER_H__
#define __MAXMINHOLDER_H__

#include <limits>

template <typename T>
class MaxHolder
{
  T _max;

public:
  MaxHolder();
  void reset();
  bool update(T val);
  T max() const;
};
template <typename T>
MaxHolder<T>::MaxHolder()
    : _max(std::numeric_limits<T>::lowest())
{
}
template <typename T>
void MaxHolder<T>::reset()
{
  _max = std::numeric_limits<T>::lowest();
}
template <typename T>
bool MaxHolder<T>::update(T val)
{
  bool isUpdated = false;
  if (_max < val)
  {
    _max = val;
    isUpdated = true;
  }
  return isUpdated;
}
template <typename T>
T MaxHolder<T>::max() const
{
  return _max;
}

////////////////////////////////////////////////////////////////////////////////
template <typename T>
class MinHolder
{
  T _min;

public:
  MinHolder();
  void reset();
  bool update(T val);
  T min() const;
};
template <typename T>
MinHolder<T>::MinHolder()
    : _min(std::numeric_limits<T>::max())
{
}
template <typename T>
void MinHolder<T>::reset()
{
  _min = std::numeric_limits<T>::max();
}
template <typename T>
bool MinHolder<T>::update(T val)
{
  bool isUpdated = false;
  _min = std::numeric_limits<T>::max();
  if (val < _min)
  {
    _min = val;
    isUpdated = true;
  }
  return isUpdated;
}
template <typename T>
T MinHolder<T>::min() const
{
  return _min;
}

////////////////////////////////////////////////////////////////////////////////
template <typename T>
class MaxMinHolder
{
  T _max;
  T _min;

public:
  MaxMinHolder();
  void reset();
  struct UpdateResult
  {
    enum Type
    {
      NONE = 0,
      MAX = 1,
      MIN = 2,
      MAXMIN = MAX | MIN,
    };
    Type _val;
    UpdateResult(Type val) : _val(val) {}
    bool isUpdatedMax() const { return (_val & UpdateResult::MAX) != 0; }
    bool isUpdatedMin() const { return (_val & UpdateResult::MIN) != 0; }
  };
  UpdateResult update(T val);
  T max() const;
  T min() const;
};
template <typename T>
MaxMinHolder<T>::MaxMinHolder()
    : _max(std::numeric_limits<T>::lowest()), _min(std::numeric_limits<T>::max())
{
}
template <typename T>
void MaxMinHolder<T>::reset()
{
  _max = std::numeric_limits<T>::lowest();
  _min = std::numeric_limits<T>::max();
}
template <typename T>
typename MaxMinHolder<T>::UpdateResult MaxMinHolder<T>::update(T val)
{
  UpdateResult updated = UpdateResult::NONE;
  if (val < _min)
  {
    _min = val;
    updated = static_cast<typename UpdateResult::Type>(updated._val | UpdateResult::MIN);
  }
  if (_max < val)
  {
    _max = val;
    updated = static_cast<typename UpdateResult::Type>(updated._val | UpdateResult::MAX);
  }
  return updated;
}
template <typename T>
T MaxMinHolder<T>::max() const
{
  return _max;
}
template <typename T>
T MaxMinHolder<T>::min() const
{
  return _min;
}

#endif // __MAXMINHOLDER_H__
