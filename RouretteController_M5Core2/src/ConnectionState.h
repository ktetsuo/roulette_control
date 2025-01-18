#ifndef __CONNECTIONSTATE_H__
#define __CONNECTIONSTATE_H__

enum class ConnectionState
{
    Disconnected, // 未接続
    Connecting,   // 接続中
    Connected,    // 接続済み
};

#endif // __CONNECTIONSTATE_H__
