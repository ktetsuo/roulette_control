#ifndef __EVENT_H__
#define __EVENT_H__

#include <variant>

// イベントなし
struct EvNone
{
};

// 目標数値変更イベント
struct EvChangeTargetNumber
{
    int number;
};

using Event = std::variant<
    EvNone,
    EvChangeTargetNumber>;

#endif // __EVENT_H__
