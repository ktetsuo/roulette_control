#include "RouletteEncoder.h"
#include "AS5600.h"
#include <cmath>

/// @brief エンコーダーの1周のカウント値
static constexpr unsigned int ROUND_COUNT = 4096;
/// @brief ルーレットの目の最小値
static constexpr unsigned int NUMBER_MIN = 1;
/// @brief ルーレットの目の最大値
static constexpr unsigned int NUMBER_MAX = 10;

/// @brief
/// @param currentRawPos 現在の値
/// @param lastRawPos 前回の値
/// @return
static int calcPosDiff(unsigned int currentRawPos, unsigned int lastRawPos)
{
  const int diff = static_cast<int>(currentRawPos) - static_cast<int>(lastRawPos);
  if (diff >= static_cast<int>(ROUND_COUNT) / 2)
  {
    // 1周の半分以上増えていたということはマイナス方向に動いたと思われるため修正
    return diff - ROUND_COUNT;
  }
  else if (diff < -static_cast<int>(ROUND_COUNT) / 2)
  {
    // 1周の半分以上減っていたということはプラス方向に動いたと思われるため修正
    return diff + ROUND_COUNT;
  }
  return diff;
}

RouletteEncoder::RouletteEncoder(AS5600 &as5600)
    : _as5600(as5600)
{
}

void RouletteEncoder::init()
{
  // エンコーダから初回位置を取得
  const unsigned int rawPos = getRawAngle();
  // 累積位置を原点からの距離に設定
  _totalPos = calcPosDiff(rawPos, _originPos);
  // 最終位置を更新
  _lastRawPos = rawPos;
}

void RouletteEncoder::update()
{
  // エンコーダから角度を取得
  const unsigned int currentRawPos = getRawAngle();
  // 回転量を計算
  const int posDiff = calcPosDiff(currentRawPos, _lastRawPos);
  // 累積位置を加算
  _totalPos += posDiff;
  // 前回からの差を更新
  _lastPosDiff = posDiff;
  // 最終位置を更新
  _lastRawPos = currentRawPos;
}

long RouletteEncoder::totalPos() const
{
  return _totalPos;
}

void RouletteEncoder::resetTotalPos()
{
  // 累積位置を原点からの距離に設定
  _totalPos = calcPosDiff(_lastRawPos, _originPos);
}

unsigned int RouletteEncoder::rawPos() const
{
  return _lastRawPos;
}

int RouletteEncoder::lastPosDiff() const
{
  return _lastPosDiff;
}

unsigned int RouletteEncoder::pos() const
{
  return (ROUND_COUNT + _lastRawPos - _originPos) % ROUND_COUNT;
}

unsigned int RouletteEncoder::number() const
{
  // ルーレットの目を計算
  return pos() * (NUMBER_MAX - NUMBER_MIN + 1) / ROUND_COUNT + NUMBER_MIN;
}

unsigned int RouletteEncoder::originPos() const
{
  return _originPos;
}

void RouletteEncoder::originPos(unsigned int pos)
{
  // 原点位置の差分だけ累積位置をオフセットする
  const int diff = calcPosDiff(pos, _originPos);
  _totalPos -= diff;
  // 原点位置を設定
  _originPos = pos;
}

/// @brief 数値numberにpから一番近い位置を求める
/// @param p
/// @param number
/// @return
long RouletteEncoder::nearestNumberTotalPos(long p, unsigned int number) const
{
  const long q = static_cast<long>(std::round(static_cast<float>(p) / ROUND_COUNT)); // 商
  const long pos1 = static_cast<long>(std::round((static_cast<float>(ROUND_COUNT) * q + static_cast<float>(number - NUMBER_MIN) * (ROUND_COUNT) / (NUMBER_MAX - NUMBER_MIN + 1) + static_cast<float>(ROUND_COUNT) / (NUMBER_MAX - NUMBER_MIN + 1) / 2)));
  const long pos2 = pos1 - ROUND_COUNT;
  if (labs(pos1 - p) < labs(pos2 - p))
  {
    // pos1のほうが近い
    return pos1;
  }
  else
  {
    // pos2のほうが近い
    return pos2;
  }
}

unsigned int RouletteEncoder::getRawAngle() const
{
  // デバイスから角度を取得
  const unsigned int angle = _as5600.getRawAngle();
  // デバイスから得られる角度はルーレットの目と方向が逆なので、反転して返す

  return ROUND_COUNT - angle - 1;
}
