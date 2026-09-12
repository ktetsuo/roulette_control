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
    : _display(display), _circleSprite(&display)
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
    _circleSprite.createSprite(_circleDiameter, _circleDiameter);
    _circleSprite.setPivot(_circleCenterX, _circleCenterY);
    drawRouletteBase();
    _display.clear();
    update();
}

void RouletteDisplay::update()
{
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

    float rouletteAngle = 0.f;
    if (_mode == Mode::Display)
    {
        rouletteAngle = _currentPos * 360. / 4096.;
    }
    else
    {
        rouletteAngle = 0.f;
    }
    const int16_t circleX = (_display.width() - _circleSprite.width()) / 2;
    const int16_t circleY = (_display.height() - _circleSprite.height()) / 2;
    _circleSprite.pushRotateZoom(&_display, circleX + _circleCenterX, circleY + _circleCenterY,
                                 rouletteAngle, 1.f, 1.f, TFT_BLACK);
    drawCenterNumber(_mode == Mode::Display ? _currentNumber : _targetNumber,
                     circleX + _circleCenterX, circleY + _circleCenterY);

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

void RouletteDisplay::drawRouletteBase()
{
    M5Canvas numberSprite(&_circleSprite);
    numberSprite.createSprite(64, 64);
    numberSprite.setBitmapColor(TFT_BLACK, TFT_TRANSPARENT);
    numberSprite.setBaseColor(TFT_TRANSPARENT);
    _circleSprite.fillSprite(TFT_BLACK);
    _circleSprite.setColor(TFT_WHITE);
    _circleSprite.fillEllipse(_circleCenterX, _circleCenterY, _circleRadius, _circleRadius);
    for (int i = 1; i <= 10; i++)
    {
        _circleSprite.fillArc(_circleCenterX, _circleCenterY, _circleRadius - _outerCircleBorder,
                              _innerCircleRadius + _innerCircleBorder,
                              360 + 360. / 10 * (i - 1) - 45,
                              360 + 360. / 10 * i - 45, numberColor(i));
        numberSprite.fillSprite(TFT_TRANSPARENT);
        numberSprite.setFont(&fonts::Font8);
        numberSprite.setTextSize(1);
        const int16_t x = (numberSprite.width() - numberSprite.textWidth(String(i))) / 2;
        const int16_t y = (numberSprite.height() - numberSprite.fontHeight()) / 2 + 5;
        numberSprite.setTextColor(TFT_WHITE);
        numberSprite.setCursor(x, y);
        numberSprite.print(i);
        numberSprite.setPivot(numberSprite.width() / 2, numberSprite.height() / 2);
        const float numberAngle = 36.f * (i - 1) * 3.14159265f / 180.f;
        const int numberX = _circleCenterX +
                            static_cast<int>((_numberPosX - _circleCenterX) * std::cos(numberAngle) -
                                             (_numberPosY - _circleCenterY) * std::sin(numberAngle));
        const int numberY = _circleCenterY +
                            static_cast<int>((_numberPosX - _circleCenterX) * std::sin(numberAngle) +
                                             (_numberPosY - _circleCenterY) * std::cos(numberAngle));
        numberSprite.pushRotateZoom(numberX, numberY,
                                    90 + 18 + 36 * (i - 1) - 45,
                                    1.f, 1.f, TFT_TRANSPARENT);
    }
}

void RouletteDisplay::drawCenterNumber(int centerNumber, int16_t centerX, int16_t centerY)
{
    _display.setColor(numberColor(centerNumber));
    _display.fillEllipse(centerX, centerY, _innerCircleRadius, _innerCircleRadius);
    _display.setFont(&fonts::Font8);
    _display.setTextSize(2);
    const int16_t x = centerX - _display.textWidth(String(centerNumber)) / 2;
    const int16_t y = centerY - _display.fontHeight() / 2 + 5;
    _display.setTextColor(TFT_BLACK);
    _display.setCursor(x + 1, y + 1);
    _display.print(centerNumber);
    _display.setCursor(x + 1, y - 1);
    _display.print(centerNumber);
    _display.setCursor(x - 1, y + 1);
    _display.print(centerNumber);
    _display.setCursor(x - 1, y - 1);
    _display.print(centerNumber);
    _display.setTextColor(TFT_WHITE);
    _display.setCursor(x, y);
    _display.print(centerNumber);
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