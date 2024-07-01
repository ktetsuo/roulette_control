#ifndef __ROULETTEENCODER_H__
#define __ROULETTEENCODER_H__

class AS5600;

class RouletteEncoder
{
  AS5600 &_as5600;
  /// @brief 累積位置
  long _totalPos = 0;
  /// @brief 目と累積位置の原点位置（目が10から1に切り替わる点）
  unsigned int _originPos = 0;
  /// @brief 最終位置
  unsigned int _lastRawPos = 0;
  /// @brief 前回からの差分
  int _lastPosDiff = 0;

public:
  RouletteEncoder(AS5600 &as5600);
  void init();
  void update();
  long totalPos() const;
  void resetTotalPos();
  unsigned int rawPos() const;
  int lastPosDiff() const;
  unsigned int pos() const;
  unsigned int number() const;
  unsigned int originPos() const;
  void originPos(unsigned int pos);
  long nearestNumberTotalPos(long p, unsigned int number) const;

private:
  unsigned int getRawAngle() const;
};

#endif // __ROULETTEENCODER_H__
