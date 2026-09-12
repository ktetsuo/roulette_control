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
    void drawRouletteBase();
    void drawCenterNumber(int centerNumber, int16_t centerX, int16_t centerY);
    Event onTouchedCenterCircle();
    Event onTouchedNumber(int number);

private:
    M5GFX &_display;
    M5Canvas _circleSprite;
    int _targetNumber = 1;
    int _currentNumber = 1;
    int _currentPos = 0;
    int _currentSpeed = 0;
    ConnectionState _connectionState = ConnectionState::Disconnected;

    enum class Mode
    {
        Display,
        Setting,
    };
    Mode _mode = Mode::Display;
    unsigned long _lastDisplayTime = 0;
    unsigned long _lastToggleTime = 0;
    bool _toggleOn = true;
};

#endif // __ROULETTEDISPLAY_H__