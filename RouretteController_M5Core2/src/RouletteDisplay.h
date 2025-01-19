#ifndef __ROULETTEDISPLAY_H__
#define __ROULETTEDISPLAY_H__

#include <M5Unified.h>
#include "ConnectionState.h"

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

    void onTouched(int x, int y);

private:
    void drawRoulette(int centerNumber, float angle);
    void onTouchedCenterCircle();
    void onTouchedNumber(int number);

private:
    M5GFX &_display;
    M5Canvas _numberSprite;
    M5Canvas _circleSprite;
    int _targetNumber = 1;                                            // 目標の数値
    int _currentNumber = 1;                                           // 現在の数値
    int _currentPos = 0;                                              // 現在の角度位置
    ConnectionState _connectionState = ConnectionState::Disconnected; // 接続状態

    enum class Mode
    {
        Display, // 表示モード
        Setting, // 設定モード
    };
    Mode _mode = Mode::Display;
};

#endif // __ROULETTEDISPLAY_H__
