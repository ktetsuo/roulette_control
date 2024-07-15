#include <Arduino.h>
#include <Wire.h>
#include "Timer.h"
#include "MaxMinHolder.h"
#include "DigitalOut.h"
#include "BootSelIn.h"
#include "ChatteringFilteredDigitalInWatcher.h"
#include "AS5600.h"
#include "ValueChangeWatcher.h"
#include "IntervalWatcher.h"
#include "HeartBeatLED.h"
#include "PWM.h"

#include "Duty.h"
#include "TB6612.h"

#include "SerialConsole.h"
#include "ValueCommand.h"
#include "SimpleCommand.h"

#include "RotationBuffer.h"
#include "RouletteEncoder.h"

static DigitalOut tb6612In1(19);
static DigitalOut tb6612In2(20);
static PWM tb6612Pwm(21, 1000, 20000);
static TB6612 tb6612(tb6612In1, tb6612In2, tb6612Pwm);

static BootSelIn bootSelIn;
static ChatteringFilteredDigitalInWatcher bootSelWatcher(bootSelIn, 50, false);

static AS5600 as5600(Wire);
static RouletteEncoder rouletteEncoder(as5600);
static ValueChangeWatcher<unsigned int> encoderChangeWatcher;

static DigitalOut boardLED(LED_BUILTIN);
static HeartBeatLED heartBeatLED(boardLED, 1000);

static ValueChangeWatcher<int> encoderSpeedWatcher;

static RotationBuffer<int, 100> speedBuf;

static repeating_timer motorControlTimer;

// PWM=100のときの加速度
static constexpr float accelPwm100 = 1.77f;
// モーターフリーのときの自然加速度（減速度）の絶対値
static constexpr float accelFreeAbs = 0.452f;
// 速度がvのときの自然加速度（減速度）
static constexpr float accelFree(float v)
{
  // 速度の符号と逆方向になる
  if (0 < v)
  {
    return -accelFreeAbs;
  }
  else if (v < 0)
  {
    return accelFreeAbs;
  }
  return 0.f;
}
// PWM=pのときの加速度
static constexpr float accelFromPwm(float p)
{
  return accelPwm100 / 100.f * p;
}
// 加速度=aにしたいときのPWM値
static constexpr float pwmFromAccel(float a)
{
  return a / accelPwm100 * 100.f;
}
// 速度v, 加速度aのときの停止距離を計算
static constexpr std::optional<float> calcStopLength(float v, float a)
{
  if (-0.001 < a && a < 0.001)
  {
    return std::nullopt;
  }
  return v * v / a / 2.f;
}
// 位置x, 速度v, 加速度aのときの停止位置を計算
static constexpr std::optional<float> calcStopPos(float x, float v, float a)
{
  if (-0.001 < a && a < 0.001)
  {
    return std::nullopt;
  }
  return x - v * v / a / 2.f;
}

struct LogData
{
  unsigned int rawPos;
  unsigned int pos;
  unsigned int number;
  long totalPos;
  float speedAverage;
  float accelAverage;
  float estimatedStopPos;
  float targetStopPos;
  float remainLength;
  float targetAccel;
  float relativeAccel;
  float pwmValue;
};
static const LogData initialLogData = {0};
static RotationBuffer<LogData, 500, &initialLogData> logBuf;
static bool isLogging = false;

static int pwmGetFunc()
{
  return 0; // dummy
}
static void pwmSetFunc(int val)
{
  tb6612.drive(val);
}
static unsigned int originGetFunc()
{
  return rouletteEncoder.originPos();
}
static void originSetFunc(unsigned int val)
{
  rouletteEncoder.originPos(val);
}
static void reboot()
{
  rp2040.reboot();
}
static void startLog()
{
  const uint32_t interrupt = save_and_disable_interrupts(); // 割り込み禁止
  isLogging = true;
  restore_interrupts(interrupt); // 割り込み禁止解除
}
static void endLog()
{
  const uint32_t interrupt = save_and_disable_interrupts(); // 割り込み禁止
  isLogging = false;
  restore_interrupts(interrupt); // 割り込み禁止解除
}
static void printLog()
{
  for (const LogData &log : logBuf)
  {
    Serial.printf("%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f\n",
                  log.pos,
                  log.rawPos,
                  log.number,
                  log.totalPos,
                  log.speedAverage,
                  log.accelAverage,
                  log.estimatedStopPos,
                  log.targetStopPos,
                  log.remainLength,
                  log.targetAccel,
                  log.relativeAccel,
                  log.pwmValue);
  }
}
static void clearLog()
{
  logBuf.clear();
}

static int g_accel = 0;
static void accelSetFunc(int a)
{
  g_accel = a;
}
static int accelGetFunc()
{
  return g_accel;
}

static int g_measurePWM = 0;
static bool g_startMeasure = false;
static int measureGetFunc()
{
  return g_measurePWM;
}
static void measureSetFunc(int p)
{
  g_measurePWM = p;
  // ついでに計測開始
  g_startMeasure = true;
}

static unsigned int g_target = 1;
static unsigned int targetGetFunc()
{
  return g_target;
}
static void targetSetFunc(unsigned int target)
{
  g_target = std::clamp(target, 1U, 10U);
}

static void resetOrigin()
{
  const unsigned int rawPos = rouletteEncoder.rawPos();
  rouletteEncoder.originPos(rawPos);
  Serial.printf("resetOrigin: %u\n", rawPos);
}

static void printPos()
{
  Serial.printf("pos: %u\n", rouletteEncoder.pos());
}

static ValueCommand<int> pwmCommand("pwm", pwmGetFunc, pwmSetFunc);
static SimpleCommand printPosCommand("p", printPos);
static SimpleCommand rebootCommand("r", reboot);
static ValueCommand<unsigned int> originCommand("origin", originGetFunc, originSetFunc);
static SimpleCommand startLogCommand("s", startLog);
static SimpleCommand endLogCommand("e", endLog);
static ValueCommand<int> accelCommand("accel", accelGetFunc, accelSetFunc);
static ValueCommand<int> measureCommand("measure", measureGetFunc, measureSetFunc);
static ValueCommand<unsigned int> targetCommand("target", targetGetFunc, targetSetFunc);
static SimpleCommand resetOriginCommand("ro", resetOrigin);

static constexpr size_t CMD_NUM = 10;
static std::array<const IConsoleCommand *, CMD_NUM> consoleCommands = {
    &pwmCommand,
    &printPosCommand,
    &rebootCommand,
    &originCommand,
    &startLogCommand,
    &endLogCommand,
    &accelCommand,
    &measureCommand,
    &targetCommand,
    &resetOriginCommand,
};

static SerialConsole<CMD_NUM> serialConsole(Serial, consoleCommands);

enum class State
{
  STOP,              // 停止中
  WAIT_DECELERATION, // 減速待ち
  CONTROLING,        // 制御中
  LAST_BRAKE,        // 最後のブレーキ
  //
  MEASURE,       // 加速度計測中
  MEASURE_AFTER, // 加速度計測後の減速待ち
};
static State state = State::STOP;

/// @brief モーター制御関数（割り込みで定期的に呼ばれる）
/// @param t タイマー構造体のポインタ
/// @return 常にtrue（falseを返すと割り込みが停止してしまう）
bool motorControlHandler(repeating_timer *t)
{
  const unsigned long t0 = micros();
  digitalWrite(LED_BUILTIN, HIGH);
  rouletteEncoder.update();
  // 平均速度を求める
  static constexpr size_t SPEED_BUF_SIZE = 25;
  static int speedBuf[SPEED_BUF_SIZE] = {};
  static size_t speedBufIndex = 0;
  static int speedSum = 0;
  static constexpr size_t SPEED_BUF_SHORT_SIZE = 5;
  static int speedBufShort[SPEED_BUF_SHORT_SIZE] = {};
  static size_t speedBufShortIndex = 0;
  static int speedShortSum = 0;

  const int speed = rouletteEncoder.lastPosDiff();

  speedSum -= speedBuf[speedBufIndex];
  speedSum += speed;
  const float speedAverage = static_cast<float>(speedSum) / SPEED_BUF_SIZE;
  speedBuf[speedBufIndex] = speed;
  speedBufIndex = (speedBufIndex + 1) % SPEED_BUF_SIZE;

  speedShortSum -= speedBufShort[speedBufShortIndex];
  speedShortSum += speed;
  const float speedShortAverage = static_cast<float>(speedShortSum) / SPEED_BUF_SHORT_SIZE;
  speedBufShort[speedBufShortIndex] = speed;
  speedBufShortIndex = (speedBufShortIndex + 1) % SPEED_BUF_SHORT_SIZE;
  // 平均加速度を求める
  static constexpr size_t ACCEL_BUF_SIZE = 25;
  static int accelBuf[ACCEL_BUF_SIZE] = {};
  static int lastSpeed = 0;
  static size_t accelBufIndex = 0;
  static int accelSum = 0;
  const int accel = speed - lastSpeed;
  lastSpeed = speed;
  accelSum -= accelBuf[accelBufIndex];
  accelSum += accel;
  const float accelAverage = static_cast<float>(accelSum) / ACCEL_BUF_SIZE;
  accelBuf[accelBufIndex] = accel;
  accelBufIndex = (accelBufIndex + 1) % ACCEL_BUF_SIZE;
  // 推定停止位置を求める
  static float lastEstimatedStopPos = rouletteEncoder.totalPos();
  const auto estimatedStopPosResult = calcStopPos(rouletteEncoder.totalPos(), speedShortAverage, accelFree(speedShortAverage));
  const float estimatedStopPos = estimatedStopPosResult.value_or(lastEstimatedStopPos); // ゼロ除算が発生する場合は仕方ないので前回の値を使う
  lastEstimatedStopPos = estimatedStopPos;
  //
  static float targetStopPos = 0.f; // 目標の位置
  float remainLength = 0.f;
  float targetAccel = 0.f;
  float relativeAccel = 0.f;
  float pwmValue = 0.f;

  // 状態遷移
  switch (state)
  {
  case State::STOP:
    if (150 < fabs(speedAverage))
    {
      // 平均速度が一定以上になったら減速待ちに遷移
      state = State::WAIT_DECELERATION;
      targetStopPos = 0.f;
    }
    if (g_startMeasure)
    {
      // 計測フラグが立っていたら計測開始
      g_startMeasure = false;
      tb6612.drive(g_measurePWM);
      isLogging = true;
      state = State::MEASURE;
    }
    break;
  case State::WAIT_DECELERATION:
    if (fabs(speedAverage) <= 100)
    {
      // 平均速度が一定以下になったら制御状態へ遷移
      isLogging = true;
      targetStopPos = rouletteEncoder.nearestNumberTotalPos(estimatedStopPos, g_target); // 目標位置を固定
      tb6612.drive(g_accel);
      state = State::CONTROLING;
    }
    break;
  case State::CONTROLING:
    if (true)
    {
      // 目標位置までの距離を計算
      remainLength = targetStopPos - rouletteEncoder.totalPos();
      if (fabs(remainLength) < 100.f)
      {
        // 目標位置到達
        tb6612.brake();
        state = State::LAST_BRAKE;
      }
      else
      {
        // 現在の速度から目標位置でちょうど停止するための加速度を計算
        targetAccel = -speedShortAverage * speedShortAverage / remainLength / 2.f;
        // 自然減速からの相対加速度を計算
        relativeAccel = targetAccel - accelFree(speedShortAverage);
        // 相対加速度を出すためのPWM値を計算
        pwmValue = pwmFromAccel(relativeAccel);
        // PWM出力
        tb6612.drive(pwmValue);
      }
    }
    if (fabs(speedShortAverage) <= 10)
    {
      tb6612.brake();
      // tb6612.drive(100);
      // isLogging = false;
      state = State::LAST_BRAKE;
    }
    break;
  case State::LAST_BRAKE:
    if (fabs(speedShortAverage) <= 1)
    {
      tb6612.drive(0);
      isLogging = false;
      state = State::STOP;
    }
    break;
  case State::MEASURE:
    if (logBuf.isFilled())
    {
      // ログバッファがいっぱいになったら計測停止
      tb6612.drive(0);
      isLogging = false;
      state = State::MEASURE_AFTER;
    }
    break;
  case State::MEASURE_AFTER:
    if (fabs(speedShortAverage) <= 1)
    {
      // 減速完了
      tb6612.drive(0);
      state = State::STOP;
    }
    break;
  default:
    break;
  }

  if (isLogging)
  {
    const LogData logData = {
        rouletteEncoder.rawPos(),
        rouletteEncoder.pos(),
        rouletteEncoder.number(),
        rouletteEncoder.totalPos(),
        speedAverage,
        accelAverage,
        estimatedStopPos,
        targetStopPos,
        remainLength,
        targetAccel,
        relativeAccel,
        pwmValue,
    };
    logBuf.add(logData);
  }
  digitalWrite(LED_BUILTIN, LOW);
  return true;
}

void setup()
{
  heartBeatLED.setup();
  Serial.begin(115200);
  Wire.setSDA(4);
  Wire.setSCL(5);
  Wire.begin();
  Wire.setClock(1000000);
  as5600.begin();
  rouletteEncoder.init();
  tb6612In1.setup();
  tb6612In2.setup();
  tb6612Pwm.setup();
  tb6612.setup();
  // タイマー割り込み開始
  if (!add_repeating_timer_ms(4, motorControlHandler, nullptr, &motorControlTimer))
  {
    Serial.println("error: add_repeating_timer_ms");
  }
}

void loop()
{
  static IntervalWatcher intervalWatcher("loop", Serial);
  intervalWatcher.update();
  // heartBeatLED.loop();
  static MilliSecTimeoutTimer bootSelContinueTimer(2000);
  bootSelWatcher.update();
  if (bootSelWatcher.isRisingEdge())
  {
    Serial.println("BOOTSEL ON!");
    bootSelContinueTimer.reset();
  }
  else if (bootSelWatcher.isFallingEdge())
  {
    Serial.println("BOOTSEL OFF!");
    if (bootSelContinueTimer.isTimeout())
    {
      // 2秒以上押されていたら再起動
      // 再起動したときにBOOTSELが押されているとBOOTに入ってしまうので、2秒経った瞬間ではなく離したタイミングとする
      Serial.println("Reboot!");
      Serial.end();
      cancel_repeating_timer(&motorControlTimer);
      delay(1000);
      rp2040.reboot();
    }
  }
  const uint32_t interrupt = save_and_disable_interrupts(); // 割り込み禁止
  MicroSecLapTimer disableInterruptLapTimer;
  disableInterruptLapTimer.reset();
  const unsigned int rawPos = rouletteEncoder.rawPos();
  const unsigned int pos = rouletteEncoder.pos();
  const unsigned int number = rouletteEncoder.number();
  const long totalPos = rouletteEncoder.totalPos();
  const int lastPosDiff = rouletteEncoder.lastPosDiff();
  const State currentState = state;
  disableInterruptLapTimer.lap();
  restore_interrupts(interrupt); // 割り込み禁止解除
  // 状態変化を監視
  static State lastState = State::STOP;
  if (currentState != lastState)
  {
    switch (currentState)
    {
    case State::STOP:
      Serial.println("STOP");
      printLog();
      clearLog();
      break;
    case State::WAIT_DECELERATION:
      Serial.println("WAIT_DECELERATION");
      break;
    case State::CONTROLING:
      Serial.println("CONTROLLING");
      break;
    case State::LAST_BRAKE:
      Serial.println("LAST_BRAKE");
      break;
    case State::MEASURE:
      Serial.println("MEASURE");
      break;
    case State::MEASURE_AFTER:
      Serial.println("MEASURE_AFTER");
      break;
    default:
      break;
    }
    lastState = currentState;
  }
  static ValueChangeWatcher<unsigned int> numberWatcher;
  // numberWatcher.update(number);
  static ValueChangeWatcher<long> totalPosWatcher;
  // totalPosWatcher.update(totalPos / 10);
  if (numberWatcher.isChanged() || totalPosWatcher.isChanged())
  {
    Serial.print("Number: ");
    Serial.println(number);
    Serial.print("RawPos: ");
    Serial.println(rawPos);
    Serial.print("Pos: ");
    Serial.println(pos);
    Serial.print("TotalPos: ");
    Serial.println(totalPos);
    Serial.print("LastPosDiff: ");
    Serial.println(lastPosDiff);
  }
  if (false)
  {
    Serial.print("DisableInterruptTime: ");
    Serial.println(disableInterruptLapTimer.lapTime());
  }
  serialConsole.run();
}
