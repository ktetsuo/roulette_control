#ifndef __XSEMAPHORE_H__
#define __XSEMAPHORE_H__

#include <freertos/semphr.h>

// 前方宣言
template <typename T>
class XSemaphoreLock;

template <typename T>
class XProtectVariable
{
    friend class XSemaphoreLock<T>;

private:
    xSemaphoreHandle _handle;
    T _val;

public:
    XProtectVariable(const T &initialValue)
        : _val(initialValue)
    {
        _handle = xSemaphoreCreateMutex();
    }
    ~XProtectVariable()
    {
        if (_handle != nullptr)
        {
            vSemaphoreDelete(_handle);
        }
    }
};

template <typename T>
class XSemaphoreLock
{
public:
    XSemaphoreLock(XProtectVariable<T> &variable)
        : _variable(variable)
    {
        xSemaphoreTake(variable._handle, portMAX_DELAY);
    }
    ~XSemaphoreLock()
    {
        xSemaphoreGive(_variable._handle);
    }
    T &getRef()
    {
        return _variable._val;
    }
    const T &getRef() const
    {
        return _variable._val;
    }

private:
    XProtectVariable<T> &_variable;
};

#endif // __XSEMAPHORE_H__
