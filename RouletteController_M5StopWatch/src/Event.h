#ifndef __EVENT_H__
#define __EVENT_H__

#include <variant>

struct EvNone
{
};

struct EvChangeTargetNumber
{
    int number;
};

using Event = std::variant<
    EvNone,
    EvChangeTargetNumber>;

#endif // __EVENT_H__