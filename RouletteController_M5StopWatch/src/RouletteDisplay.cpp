#include "RouletteDisplay.h"
#include <M5Unified.h>
#include <cmath>

static constexpr int _circleDiameter = 400;
static constexpr int _innerCircleDiameter = 180;
static constexpr int _outerCircleBorder = 10;
static constexpr int _innerCircleBorder = 20;

static constexpr int _circleRadius = _circleDiameter / 2 - 1;
static constexpr int _innerCircleRadius = _innerCircleDiameter / 2 - 1;
static constexpr int _circleCenterX = _circleDiameter / 2 - 1;
static constexpr int _circleCenterY = _circleDiameter / 2 - 1;
static constexpr float COS_18 = 0.9510565f; // cos(18 degrees)
static constexpr float SIN_18 = 0.3090170f; // sin(18 degrees)
static constexpr int _numberPosX = _circleCenterX + (_circleRadius - _outerCircleBorder + _innerCircleRadius + _innerCircleBorder) / 2 * COS_18;
static constexpr int _numberPosY = _circleCenterY - (_circleRadius - _outerCircleBorder + _innerCircleRadius + _innerCircleBorder) / 2 * SIN_18;

static uint32_t numberColor(int number)
{
    static constexpr uint32_t color_table[] = {
        0xFFFF00,
        0xFFD700,
        0xFF8C00,
        0xFF0000,
        0xFF00FF,
        0x800080,
        0x0000CD,
        0x4169E1,
        0x008000,
        0x9ACD32,
    };
    if (number < 1 || 10 < number)
    {
        return 0x808080;
    }
    return color_table[number - 1];
}

RouletteDisplay::RouletteDisplay(M5GFX &display)
    : _display(display), _circleSprite(&display), _numberSprite(&_circleSprite)
{
}

RouletteDisplay::~RouletteDisplay()
{
}

void RouletteDisplay::init()
{
    _lastToggleTime = millis();
    _lastDisplayTime = millis();
    _display.setBrightness(255);
    _display.setRotation(0);
    _numberSprite.createSprite(64, 64);
    _numberSprite.setBitmapColor(TFT_BLACK, TFT_TRANSPARENT);
    _numberSprite.setBaseColor(TFT_TRANSPARENT);
    _circleSprite.createSprite(_circleDiameter, _circleDiameter);
    _circleSprite.setPivot(_circleCenterX, _circleCenterY);
    _display.clear();
    update();
}

void RouletteDisplay::update()
{
    const unsigned long ms = millis();
    if (3 <= abs(_currentSpeed))
    {
        _mode = Mode::Display;
        _lastDisplayTime = ms;
    }
    else if ((_mode == Mode::Display) && (1000 < ms - _lastDisplayTime))
    {
        _mode = Mode::Setting;
    }

    _display.startWrite();
    _display.setFont(&fonts::Font0);
    _display.setTextColor(TFT_WHITE, TFT_BLACK);
    _display.setTextSize(1);
    _display.setCursor(226, 312 - 1);
    switch (_connectionState)
    {
    case ConnectionState::Disconnected:
        _display.print("--");
        break;
    case ConnectionState::Connecting:
        _display.print("..");
        break;
    case ConnectionState::Connected:
        _display.print("OK");
        break;
    }

    _circleSprite.fillSprite(TFT_BLACK);
    float rouletteAngle = 0.f;
    if (_mode == Mode::Display)
    {
        rouletteAngle = _currentPos * 360. / 4096.;
        drawRoulette(_currentNumber, true);
    }
    else
    {
        drawRoulette(_targetNumber, false);
    }
    const int16_t circleX = (_display.width() - _circleSprite.width()) / 2;
    const int16_t circleY = (_display.height() - _circleSprite.height()) / 2;
    _circleSprite.pushRotateZoom(&_display, circleX + _circleCenterX, circleY + _circleCenterY,
                                 rouletteAngle, 1.f, 1.f, TFT_BLACK);

    if (1000 <= ms - _lastToggleTime)
    {
        _lastToggleTime = ms;
        _toggleOn = !_toggleOn;
    }
    if (_mode == Mode::Setting && _toggleOn)
    {
        _display.setFont(&fonts::Font4);
        _display.setCursor(16, 7);
        _display.setTextColor(TFT_WHITE, TFT_BLACK);
        _display.setTextSize(1);
        _display.print("SELECT NUMBER");
    }
    else
    {
        _display.fillRect(0, 0, 240, 40, TFT_BLACK);
    }
    _display.endWrite();
}

void RouletteDisplay::drawRoulette(int centerNumber, bool drawNeedle)
{
    _circleSprite.setColor(TFT_WHITE);
    _circleSprite.fillEllipse(_circleCenterX, _circleCenterY, _circleRadius, _circleRadius);
    _circleSprite.setColor(numberColor(centerNumber));
    _circleSprite.fillEllipse(_circleCenterX, _circleCenterY, _innerCircleRadius, _innerCircleRadius);
    for (int i = 1; i <= 10; i++)
    {
        _circleSprite.fillArc(_circleCenterX, _circleCenterY, _circleRadius - _outerCircleBorder,
                              _innerCircleRadius + _innerCircleBorder,
                              360 + 360. / 10 * (i - 1) - 45,
                              360 + 360. / 10 * i - 45, numberColor(i));
        _numberSprite.fillSprite(TFT_TRANSPARENT);
        _numberSprite.setFont(&fonts::Font8);
        _numberSprite.setTextSize(1);
        const int16_t x = (_numberSprite.width() - _numberSprite.textWidth(String(i))) / 2;
        const int16_t y = (_numberSprite.height() - _numberSprite.fontHeight()) / 2 + 5;
        _numberSprite.setTextColor(TFT_WHITE);
        _numberSprite.setCursor(x, y);
        _numberSprite.print(i);
        _numberSprite.setPivot(_numberSprite.width() / 2, _numberSprite.height() / 2);
        const float numberAngle = 36.f * (i - 1) * 3.14159265f / 180.f;
        const int numberX = _circleCenterX +
                            static_cast<int>((_numberPosX - _circleCenterX) * std::cos(numberAngle) -
                                             (_numberPosY - _circleCenterY) * std::sin(numberAngle));
        const int numberY = _circleCenterY +
                            static_cast<int>((_numberPosX - _circleCenterX) * std::sin(numberAngle) +
                                             (_numberPosY - _circleCenterY) * std::cos(numberAngle));
        _numberSprite.pushRotateZoom(numberX, numberY,
                                     90 + 18 + 36 * (i - 1) - 45,
                                     1.f, 1.f, TFT_TRANSPARENT);
    }
    _circleSprite.setFont(&fonts::Font8);
    _circleSprite.setTextSize(2);
    const int16_t x = (_circleSprite.width() - _circleSprite.textWidth(String(centerNumber))) / 2;
    const int16_t y = (_circleSprite.height() - _circleSprite.fontHeight()) / 2 + 5;
    _circleSprite.setTextColor(TFT_BLACK);
    _circleSprite.setCursor(x + 1, y + 1);
    _circleSprite.print(centerNumber);
    _circleSprite.setCursor(x + 1, y - 1);
    _circleSprite.print(centerNumber);
    _circleSprite.setCursor(x - 1, y + 1);
    _circleSprite.print(centerNumber);
    _circleSprite.setCursor(x - 1, y - 1);
    _circleSprite.print(centerNumber);
    _circleSprite.setTextColor(TFT_WHITE);
    _circleSprite.setCursor(x, y);
    _circleSprite.print(centerNumber);
    if (drawNeedle)
    {
        // const int needleTip = static_cast<int>(10 * scale);
        // const int needleLength = static_cast<int>(60 * scale);
        // _circleSprite.fillTriangle(size - needleTip, 0, size, needleTip,
        //                            size - needleLength, needleLength, TFT_WHITE);
        // _circleSprite.drawTriangle(size - needleTip, 0, size, needleTip,
        //                            size - needleLength, needleLength, TFT_DARKGRAY);
    }
}

void RouletteDisplay::setTargetNumber(int number) { _targetNumber = number; }
void RouletteDisplay::setConnectionState(ConnectionState state) { _connectionState = state; }
void RouletteDisplay::setCurrentPos(int pos) { _currentPos = pos; }
void RouletteDisplay::setCurrentSpeed(int speed) { _currentSpeed = speed; }
void RouletteDisplay::setCurrentNumber(int number) { _currentNumber = number; }

Event RouletteDisplay::onTouched(int, int)
{
    return EvNone{};
}

Event RouletteDisplay::onTouchedCenterCircle()
{
    return EvNone{};
}

Event RouletteDisplay::onTouchedNumber(int)
{
    return EvNone{};
}