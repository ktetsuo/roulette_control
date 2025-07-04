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

// 指定角度に入っているか調べる
// angle1とangle2は必ず右回り（角度が増える方向）とする
static bool isInsideAngle(float angle, float angle1, float angle2)
{
    // 角度を0-360度の範囲に調整
    if (angle < 0)
    {
        angle += 360;
    }
    if (angle1 < 0)
    {
        angle1 += 360;
    }
    if (angle2 < 0)
    {
        angle2 += 360;
    }

    // angle1がangle2より大きい場合、angle2を360度加算して範囲を調整
    if (angle1 > angle2)
    {
        angle2 += 360;
    }
    // angleがangle1より小さい場合、360度加算して範囲を調整
    if (angle < angle1)
    {
        angle += 360;
    }

    return angle1 <= angle && angle <= angle2;
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
    _lastToggleTime = millis();
    _lastDisplayTime = millis();
    _display.setBrightness(255);
    _display.setRotation(0);
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
    // 現在時刻を取得
    const unsigned long ms = millis();
    // スピードが一定以上のときは表示モードに切り替える
    if (3 <= abs(_currentSpeed))
    {
        _mode = Mode::Display;
        // 最後に表示モードに切り替えた時間を記録
        _lastDisplayTime = ms;
    }
    else
    {
        // スピードが一定以下の時間が一定時間経過したら設定モードに切り替える
        if ((_mode == Mode::Display) && (1000 < ms - _lastDisplayTime))
        {
            _mode = Mode::Setting;
        }
    }

    // 描画開始
    _display.startWrite();
    // 接続状態
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
    // ルーレット描画
    _circleSprite.fillSprite(TFT_BLACK);
    switch (_mode)
    {
    case Mode::Display:
        drawRoulette(_currentNumber, _currentPos * 360. / 4096., true);
        break;
    case Mode::Setting:
        drawRoulette(_targetNumber, 0, false);
        break;
    default:
        break;
    }
    // LCDに転送
    _circleSprite.pushSprite(0, 40);
    // 状態表示
    if (1000 <= ms - _lastToggleTime)
    {
        _lastToggleTime = ms;
        _toggleOn = !_toggleOn;
    }
    switch (_mode)
    {
    case Mode::Setting:
        if (_toggleOn)
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
        break;
    case Mode::Display:
    default:
        _display.fillRect(0, 0, 240, 40, TFT_BLACK);
        break;
    }
    // 描画終了
    _display.endWrite();
}

// ルーレット描画
void RouletteDisplay::drawRoulette(int centerNumber, float angle, bool drawNeedle)
{
    // 定数を事前に計算
    const int centerX = 120 - 1; // 中心X座標
    const int centerY = 120 - 1; // 中心Y座標
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
        _circleSprite.fillArc(centerX, centerY, 120 - 1 - 5, 60 - 1, 360 + 360. / 10 * (i - 1) - 45 - angle, 360 + 360. / 10 * i - 45 - angle, numberColor(i));
        // 各数字
        _numberSprite.fillSprite(TFT_TRANSPARENT);
        _numberSprite.setFont(&fonts::Font6);
        _numberSprite.setTextSize(1);
        const int16_t x = (_numberSprite.width() - _numberSprite.textWidth(String(i))) / 2;
        const int16_t y = (_numberSprite.height() - _numberSprite.fontHeight()) / 2 + 5; // 5は調整値
        _numberSprite.setTextColor(TFT_WHITE);
        _numberSprite.setCursor(x, y);
        _numberSprite.print(i);
        _numberSprite.setPivot(32, 120);
        _numberSprite.pushRotateZoom(centerX, centerY, 90 + 18 + 36 * (i - 1) - 45 - angle, 1., 1., TFT_TRANSPARENT);
    }
    // 中心の数値
    _circleSprite.setFont(&fonts::Font6);
    _circleSprite.setTextSize(2);
    const int16_t x = (_circleSprite.width() - _circleSprite.textWidth(String(centerNumber))) / 2;
    const int16_t y = (_circleSprite.height() - _circleSprite.fontHeight()) / 2 + 5; // 5は調整値
    _circleSprite.setTextColor(TFT_BLACK);                                           // 縁取りの色
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
    if (drawNeedle)
    {
        _circleSprite.fillTriangle(240 - 10, 0, 240, 10, 240 - 60, 60, TFT_WHITE);
        _circleSprite.drawTriangle(240 - 10, 0, 240, 10, 240 - 60, 60, TFT_DARKGRAY);
    }
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

// 現在スピードを設定
void RouletteDisplay::setCurrentSpeed(int speed)
{
    _currentSpeed = speed;
}

// 現在ナンバーを設定
void RouletteDisplay::setCurrentNumber(int number)
{
    _currentNumber = number;
}

// タッチされたときの処理
Event RouletteDisplay::onTouched(int x, int y)
{
    Event ev = EvNone{};
    constexpr int centerX = 120;
    constexpr int centerY = 160;
    constexpr int inner_radius = 50;
    constexpr int inner_radius2 = inner_radius * inner_radius; // 半径の2乗
    constexpr int outer_radius = 120;
    constexpr int outer_radius2 = outer_radius * outer_radius; // 半径の2乗
    const int dx = x - centerX;
    const int dy = y - centerY;
    const int distance2 = dx * dx + dy * dy; // 中心からの距離の2乗
    if (distance2 < inner_radius2)
    {
        // 中心の円をタッチされた
        Serial.println("Touched: CenterCircle");
        ev = onTouchedCenterCircle();
    }
    else if (distance2 < outer_radius2)
    {
        // ルーレットの目のエリアがタッチされた
        const float radian = atan2(dy, dx);
        const float angle = degrees(radian);
        Serial.print("Touched: NumberArea ");
        Serial.println(angle);
        constexpr float numberAngleTable[10] = {
            -45. + 36 * 0, // 1
            -45. + 36 * 1, // 2
            -45. + 36 * 2, // 3
            -45. + 36 * 3, // 4
            -45. + 36 * 4, // 5
            -45. + 36 * 5, // 6
            -45. + 36 * 6, // 7
            -45. + 36 * 7, // 8
            -45. + 36 * 8, // 9
            -45. + 36 * 9, // 10
        };
        for (int n = 1; n <= 10; n++)
        {
            const int i = n - 1;
            if (isInsideAngle(angle, numberAngleTable[i], numberAngleTable[(i + 1) % 10]))
            {
                Serial.print("Number: ");
                Serial.println(n);
                ev = onTouchedNumber(n);
                break;
            }
        }
    }
    return ev;
}

// 中心の円がタッチされたときの処理
Event RouletteDisplay::onTouchedCenterCircle()
{
    switch (_mode)
    {
    case Mode::Display:
        _mode = Mode::Setting;
        break;
    case Mode::Setting:
        _mode = Mode::Display;
        break;
    default:
        break;
    }
    return EvNone{}; // イベントなし
}

// 数字がタッチされたときの処理
Event RouletteDisplay::onTouchedNumber(int number)
{
    Event ev = EvNone{};
    switch (_mode)
    {
    case Mode::Display:
        break;
    case Mode::Setting:
        _targetNumber = number;
        ev = EvChangeTargetNumber{number};
        break;
    default:
        break;
    }
    return ev;
}
