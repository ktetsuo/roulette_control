#include "RouletteDisplay.h"
#include <M5Unified.h>

static uint32_t numberColor(int number)
{
    // 色テーブル
    static constexpr uint32_t color_table[] = {
        0xFFFF00, // 1 Yellow
        0xFFD700, // 2 Gold
        0xFF8C00, // 3 DarkOrange
        0xFF0000, // 4 Red
        0xFF00FF, // 5 Magenta
        0x800080, // 6 Purple
        0x0000CD, // 7 MediumBlue
        0x4169E1, // 8 RoyalBlue
        0x008000, // 9 Green
        0x9ACD32, // 10 YellowGreen
    };
    if (number < 1 || 10 < number)
    {
        return 0x808080; // Gray
    }
    return color_table[number - 1];
}

// コンストラクタ
RouletteDisplay::RouletteDisplay(M5GFX &display)
    : _display(display), _circleSprite(&display), _numberSprite(&_circleSprite)
{
}

// デストラクタ
RouletteDisplay::~RouletteDisplay()
{
}

// 初期化
void RouletteDisplay::init()
{
    _numberSprite.createSprite(64, 64);
    _numberSprite.setBitmapColor(TFT_BLACK, TFT_TRANSPARENT); // 黒を透明に設定
    _numberSprite.setBaseColor(TFT_TRANSPARENT);
    _circleSprite.createSprite(240, 240);
    _display.clear();
    update();
}

// 表示更新
void RouletteDisplay::update()
{
    int centerNumber = _currentNumber;
    // 定数を事前に計算
    const int centerX = 120 - 1; // 中心X座標
    const int centerY = 120 - 1; // 中心Y座標
    const int currentAngle = _currentPos * 360 / 4096;
    // 描画開始
    _display.startWrite();

    // 外側の円
    _circleSprite.setColor(TFT_WHITE);
    _circleSprite.fillEllipse(centerX, centerY, 120 - 1, 120 - 1);
    // 内側の円
    _circleSprite.setColor(numberColor(centerNumber));
    _circleSprite.fillEllipse(centerX, centerY, 50 - 1, 50 - 1);
    // ルーレット部
    for (int i = 1; i <= 10; i++)
    {
        // 背景の扇形
        _circleSprite.fillArc(centerX, centerY, 120 - 1 - 5, 60 - 1, 360 + 360. / 10 * (i - 1) - 45 - currentAngle, 360 + 360. / 10 * i - 45 - currentAngle, numberColor(i));
        // 各数字
        _numberSprite.fillSprite(TFT_TRANSPARENT);
        _numberSprite.setFont(&fonts::lgfxJapanGothic_40);
        _numberSprite.setTextSize(1);
        const int16_t x = (_numberSprite.width() - _numberSprite.textWidth(String(i))) / 2;
        const int16_t y = (_numberSprite.height() - _numberSprite.fontHeight()) / 2;
        _numberSprite.setTextColor(TFT_WHITE);
        _numberSprite.setCursor(x, y);
        _numberSprite.print(i);
        _numberSprite.setPivot(32, 120);
        //        _numberSprite.pushRotateZoomWithAA(centerX, centerY, 90 + 18 + 36 * (i - 1) - 45 - currentAngle, 1., 1., TFT_TRANSPARENT);
        _numberSprite.pushRotateZoom(centerX, centerY, 90 + 18 + 36 * (i - 1) - 45 - currentAngle, 1., 1., TFT_TRANSPARENT);
    }
    // 中心の数値
    _circleSprite.setFont(&fonts::lgfxJapanGothic_40);
    _circleSprite.setTextSize(2);
    int16_t x = (_circleSprite.width() - _circleSprite.textWidth(String(centerNumber))) / 2;
    int16_t y = (_circleSprite.height() - _circleSprite.fontHeight()) / 2;
    _circleSprite.setTextColor(TFT_BLACK); // 縁取りの色
    _circleSprite.setCursor(x + 1, y + 1);
    _circleSprite.print(centerNumber);
    _circleSprite.setCursor(x + 1, y - 1);
    _circleSprite.print(centerNumber);
    _circleSprite.setCursor(x - 1, y + 1);
    _circleSprite.print(centerNumber);
    _circleSprite.setCursor(x - 1, y - 1);
    _circleSprite.print(centerNumber);
    _circleSprite.setTextColor(TFT_WHITE); // フォントの色
    _circleSprite.setCursor(x, y);
    _circleSprite.print(centerNumber);
    // 矢印
    _circleSprite.fillTriangle(240 - 10, 0, 240, 10, 240 - 60, 60, TFT_WHITE);
    _circleSprite.drawTriangle(240 - 10, 0, 240, 10, 240 - 60, 60, TFT_DARKGRAY);
    // LCDに転送
    _circleSprite.pushSprite(40, 0);
    // 接続状態
    _display.setFont(&fonts::lgfxJapanGothic_16);
    _display.setTextColor(TFT_WHITE, TFT_BLACK);
    _display.setTextSize(1);
    _display.setCursor(0, 0);
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
    // 描画終了
    _display.endWrite();
}

// ターゲットの数値を設定
void RouletteDisplay::setTargetNumber(int number)
{
    _targetNumber = number;
}

// 接続状態を設定
void RouletteDisplay::setConnectionState(ConnectionState state)
{
    _connectionState = state;
}

// 現在位置を設定
void RouletteDisplay::setCurrentPos(int pos)
{
    _currentPos = pos;
}

// 現在ナンバーを設定
void RouletteDisplay::setCurrentNumber(int number)
{
    _currentNumber = number;
}
