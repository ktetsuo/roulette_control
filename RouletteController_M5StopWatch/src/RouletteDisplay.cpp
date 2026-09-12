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
static constexpr int _segmentWidth = 220;
static constexpr int _segmentHeight = 240;
static constexpr int _segmentPivotX = 0;
static constexpr int _segmentPivotY = 150;
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
    : _display(display),
      _segmentSprites{M5Canvas(&display), M5Canvas(&display), M5Canvas(&display),
                      M5Canvas(&display), M5Canvas(&display)}
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
    createSegmentSprites();
    _display.clear();
    update();
}

void RouletteDisplay::update()
{
    _display.startWrite();
    _display.fillScreen(TFT_BLACK);
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

    drawRoulette(_mode == Mode::Display ? _currentNumber : _targetNumber,
                 _mode == Mode::Display);
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

void RouletteDisplay::createSegmentSprites()
{
    for (int spriteIndex = 0; spriteIndex < 5; spriteIndex++)
    {
        M5Canvas &segment = _segmentSprites[spriteIndex];
        segment.createSprite(_segmentWidth, _segmentHeight);
        segment.setPivot(_segmentPivotX, _segmentPivotY);
        segment.setBitmapColor(TFT_BLACK, TFT_TRANSPARENT);
        segment.setBaseColor(TFT_TRANSPARENT);
        segment.fillSprite(TFT_TRANSPARENT);
        for (int numberIndex = 0; numberIndex < 2; numberIndex++)
        {
            const int number = spriteIndex * 2 + numberIndex + 1;
            const float segmentStart = -45.f + 36.f * numberIndex;
            const float segmentEnd = segmentStart + 36.f;
            segment.fillArc(_segmentPivotX, _segmentPivotY,
                            _circleRadius - _outerCircleBorder,
                            _innerCircleRadius + _innerCircleBorder,
                            segmentStart, segmentEnd, numberColor(number));

            M5Canvas numberGlyph(&segment);
            numberGlyph.createSprite(64, 64);
            numberGlyph.setBitmapColor(TFT_BLACK, TFT_TRANSPARENT);
            numberGlyph.setBaseColor(TFT_TRANSPARENT);
            numberGlyph.fillSprite(TFT_TRANSPARENT);
            numberGlyph.setFont(&fonts::Font8);
            numberGlyph.setTextSize(1);
            const int16_t x = (numberGlyph.width() - numberGlyph.textWidth(String(number))) / 2;
            const int16_t y = (numberGlyph.height() - numberGlyph.fontHeight()) / 2 + 5;
            numberGlyph.setTextColor(TFT_WHITE);
            numberGlyph.setCursor(x, y);
            numberGlyph.print(number);
            numberGlyph.setPivot(numberGlyph.width() / 2, numberGlyph.height() / 2);
            const float numberAngle = (-18.f + 36.f * numberIndex) * 3.14159265f / 180.f;
            const int numberX = _segmentPivotX + static_cast<int>(149.f * std::cos(numberAngle));
            const int numberY = _segmentPivotY + static_cast<int>(149.f * std::sin(numberAngle));
            numberGlyph.pushRotateZoom(&segment, numberX, numberY,
                                       63.f + 36.f * numberIndex,
                                       1.f, 1.f, TFT_TRANSPARENT);
        }
    }
}

void RouletteDisplay::drawRoulette(int centerNumber, bool drawNeedle)
{
    const int16_t centerX = _display.width() / 2;
    const int16_t centerY = _display.height() / 2;
    _display.setColor(TFT_WHITE);
    _display.fillEllipse(centerX, centerY, _circleRadius, _circleRadius);
    const float rouletteAngle = _mode == Mode::Display ? _currentPos * 360.f / 4096.f : 0.f;
    for (int i = 0; i < 5; i++)
    {
        _segmentSprites[i].pushRotateZoom(
            &_display, centerX, centerY, rouletteAngle + 72.f * i,
            1.f, 1.f, TFT_TRANSPARENT);
    }
    drawCenterNumber(centerNumber, centerX, centerY);
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