#include "RouletteDisplay.h"
#include <M5Unified.h>

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
    const int16_t circleSize = _display.width() < _display.height()
                                   ? _display.width()
                                   : _display.height();
    _circleSprite.createSprite(circleSize, circleSize);
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
    if (_mode == Mode::Display)
    {
        drawRoulette(_currentNumber, _currentPos * 360. / 4096., true);
    }
    else
    {
        drawRoulette(_targetNumber, 0, false);
    }
    const int16_t circleX = (_display.width() - _circleSprite.width()) / 2;
    const int16_t circleY = (_display.height() - _circleSprite.height()) / 2;
    _circleSprite.pushSprite(circleX, circleY);

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

void RouletteDisplay::drawRoulette(int centerNumber, float angle, bool drawNeedle)
{
    const float scale = _circleSprite.width() / 240.0f;
    const int size = _circleSprite.width();
    const int centerX = size / 2;
    const int centerY = size / 2;
    const int outerRadius = size / 2 - 1;
    _circleSprite.setColor(TFT_WHITE);
    _circleSprite.fillEllipse(centerX, centerY, outerRadius, outerRadius);
    _circleSprite.setColor(numberColor(centerNumber));
    _circleSprite.fillEllipse(centerX, centerY, static_cast<int>(49 * scale),
                              static_cast<int>(49 * scale));
    for (int i = 1; i <= 10; i++)
    {
        _circleSprite.fillArc(centerX, centerY, outerRadius - static_cast<int>(5 * scale),
                              static_cast<int>(59 * scale),
                              360 + 360. / 10 * (i - 1) - 45 - angle,
                              360 + 360. / 10 * i - 45 - angle, numberColor(i));
        _numberSprite.fillSprite(TFT_TRANSPARENT);
        _numberSprite.setFont(&fonts::Font6);
        _numberSprite.setTextSize(1);
        const int16_t x = (_numberSprite.width() - _numberSprite.textWidth(String(i))) / 2;
        const int16_t y = (_numberSprite.height() - _numberSprite.fontHeight()) / 2 + 5;
        _numberSprite.setTextColor(TFT_WHITE);
        _numberSprite.setCursor(x, y);
        _numberSprite.print(i);
        _numberSprite.setPivot(32, 120);
        _numberSprite.pushRotateZoom(centerX, centerY, 90 + 18 + 36 * (i - 1) - 45 - angle,
                                     scale, scale, TFT_TRANSPARENT);
    }
    _circleSprite.setFont(&fonts::Font6);
    _circleSprite.setTextSize(static_cast<uint8_t>(2 * scale + 0.5f));
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
        const int needleTip = static_cast<int>(10 * scale);
        const int needleLength = static_cast<int>(60 * scale);
        _circleSprite.fillTriangle(size - needleTip, 0, size, needleTip,
                                   size - needleLength, needleLength, TFT_WHITE);
        _circleSprite.drawTriangle(size - needleTip, 0, size, needleTip,
                                   size - needleLength, needleLength, TFT_DARKGRAY);
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