#ifndef __ROULETTEDISPLAY_H__
#define __ROULETTEDISPLAY_H__

#include <M5Unified.h>
#include "ConnectionState.h"
#include "Event.h"

class RouletteDisplay
{
public:
    RouletteDisplay(M5GFX &display);
    ~RouletteDisplay();
    void init();
    void update();

    void setTargetNumber(int number);
    void setCurrentPos(int pos);
    void setCurrentNumber(int number);
    void setConnectionState(ConnectionState state);
    void setCurrentSpeed(int speed);

    Event onTouched(int x, int y);

private:
    void drawRoulette(int centerNumber, float angle, bool drawNeedle);
    Event onTouchedCenterCircle();
    Event onTouchedNumber(int number);

private:
    M5GFX &_display;
    M5Canvas _numberSprite;
    M5Canvas _circleSprite;
    int _targetNumber = 1;                                            // 目標の数値
    int _currentNumber = 1;                                           // 現在の数値
    int _currentPos = 0;                                              // 現在の角度位置
    int _currentSpeed = 0;                                            // 現在の速度
    ConnectionState _connectionState = ConnectionState::Disconnected; // 接続状態

    enum class Mode
    {
        Display, // 表示モード
        Setting, // 設定モード
    };
    Mode _mode = Mode::Display;
    unsigned long _lastDisplayTime = 0; // 最後に表示モードに切り替えた時間
    unsigned long _lastToggleTime = 0;  // 最後に点滅を切り替えた時間
    bool _toggleOn = true;
};

#endif // __ROULETTEDISPLAY_H__
