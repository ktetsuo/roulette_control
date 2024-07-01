#ifndef __ARRAYSTRING_H__
#define __ARRAYSTRING_H__

#include <array>

template <std::size_t N>
class ArrayString
{
  std::array<char, N + 1> _array;
  std::size_t _len;

public:
  constexpr std::size_t capacity() const
  {
    return N;
  }
  constexpr std::size_t length() const
  {
    return _len;
  }
  constexpr const char *c_str() const
  {
    return _array.data();
  }
  constexpr bool isEmpty() const
  {
    return _len == 0;
  }
  constexpr bool isFull() const
  {
    return _len >= N;
  }
  void clear()
  {
    _array[0] = '\0';
    _len = 0;
  }
  bool append(char c)
  {
    if (isFull())
    {
      return false;
    }
    _array[_len] = c;
    _len++;
    _array[_len] = '\0';
    return false;
  }
  std::size_t append(const char *s, std::size_t len)
  {
    std::size_t i = 0;
    while (i < len)
    {
      if (_len + i >= N)
      {
        break;
      }
      if (s[i] == '\0')
      {
        break;
      }
      _array[_len + i] = s[i];
      i++;
    }
    _len += i;
    _array[_len] = '\0';
    return i;
  }
};

#endif // __ARRAYSTRING_H__
