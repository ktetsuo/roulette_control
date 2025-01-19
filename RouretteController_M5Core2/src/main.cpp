#include <Arduino.h>
#include <BluetoothSerial.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <M5Unified.h>
#include <etl/array.h>
#include <MsgPacketizer.h>
#include "RouletteDisplay.h"
#include "ConnectionState.h"
#include "XSemaphore.h"
#include "../../Common/MsgIndex.h"

BluetoothSerial SerialBT;
// BTAddress rouretteAddress("28:CD:C1:0E:D4:3D");
BTAddress rouretteAddress("28:CD:C1:09:50:9D");
static int s_targetNumber = 1;
static XProtectVariable<ConnectionState> s_connectionState(ConnectionState::Disconnected);

static RouletteDisplay s_display(M5.Display);

// Bluetooth接続タスク
static void connectAsyncTask(void *parameter)
{
  while (true)
  {
    if (!SerialBT.connected())
    {
      XSemaphoreLock<ConnectionState>(s_connectionState).getRef() = ConnectionState::Connecting;
      if (SerialBT.connect(rouretteAddress))
      {
        XSemaphoreLock<ConnectionState>(s_connectionState).getRef() = ConnectionState::Connected;
      }
      else
      {
        XSemaphoreLock<ConnectionState>(s_connectionState).getRef() = ConnectionState::Disconnected;
        vTaskDelay(5000 / portTICK_PERIOD_MS); // 5秒待機して再試行
      }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1秒ごとに接続状態を確認
  }
}

// 現在位置を受信したときの処理
static void OnRecievedCurrentPos(int &pos)
{
  s_display.setCurrentPos(pos);
}

// 現在ナンバーを受信したときの処理
static void OnRecievedCurrentNumber(int &number)
{
  Serial.print("CurrentNumber: ");
  Serial.println(number);
  s_display.setCurrentNumber(number);
}

// 制御状態を受信したときの処理
static void OnRecievedControlState(int &state)
{
  Serial.print("ControlState: ");
  Serial.println(state);
}

void setup()
{
  auto cfg = M5.config();
  M5.begin(cfg); // M5Unifiedの初期化
  Serial.begin(115200);
  SerialBT.begin("RouretteController", true); // Bluetooth device name
  xTaskCreate(connectAsyncTask, "ConnectAsyncTask", 4096, NULL, 0, NULL);
  s_display.init();
  MsgPacketizer::subscribe(SerialBT, static_cast<uint8_t>(MsgIndex::CurrentPos), &OnRecievedCurrentPos);
  MsgPacketizer::subscribe(SerialBT, static_cast<uint8_t>(MsgIndex::CurrentNumber), &OnRecievedCurrentNumber);
  MsgPacketizer::subscribe(SerialBT, static_cast<uint8_t>(MsgIndex::ControlState), &OnRecievedControlState);
}

void loop()
{
  M5.update();
  MsgPacketizer::update();

  // 接続状態を画面に反映
  s_display.setConnectionState(XSemaphoreLock<ConnectionState>(s_connectionState).getRef());

  // Aボタンが押されたら
  if (M5.BtnA.wasPressed())
  {
    Serial.println("A was Pressed");
    // デクリメント
    s_targetNumber = (s_targetNumber + 8) % 10 + 1;
    // 画面に表示
    s_display.setTargetNumber(s_targetNumber);
    // Bluetoothシリアルに送信
    MsgPacketizer::send(SerialBT, static_cast<uint8_t>(MsgIndex::TargetNumber), s_targetNumber);
  }
  // Bボタンが押されたら
  if (M5.BtnB.wasPressed())
  {
    Serial.println("B was Pressed");
  }
  // Cボタンが押されたら
  if (M5.BtnC.wasPressed())
  {
    Serial.println("C was Pressed");
    // インクリメント
    s_targetNumber = s_targetNumber % 10 + 1;
    // 画面に表示
    s_display.setTargetNumber(s_targetNumber);
    // Bluetoothシリアルに送信
    MsgPacketizer::send(SerialBT, static_cast<uint8_t>(MsgIndex::TargetNumber), s_targetNumber);
  }

  // タッチパネル
  const uint8_t touchCount = M5.Touch.getCount();
  static bool s_lastTouched = false;
  const bool touched = touchCount > 0;
  static lgfx::v1::touch_point_t s_lastTouchPoint;
  if (touched)
  {
    // タッチ位置取得
    const lgfx::v1::touch_point_t touchPoint = M5.Touch.getTouchPointRaw();
    if (!s_lastTouched)
    {
      // タッチ開始
      Serial.print("Touch Start! X: ");
      Serial.print(touchPoint.x);
      Serial.print(", Y: ");
      Serial.println(touchPoint.y);
      // ディスプレイに通知
      s_display.onTouched(touchPoint.x, touchPoint.y);
    }
    // 最終タッチ位置更新
    s_lastTouchPoint = touchPoint;
  }
  else
  {
    if (s_lastTouched)
    {
      // タッチ終了
      Serial.print("Touch End! X: ");
      Serial.print(s_lastTouchPoint.x);
      Serial.print(", Y: ");
      Serial.println(s_lastTouchPoint.y);
    }
  }
  // 最終タッチ状態更新
  s_lastTouched = touched;

  // 画面の更新
  s_display.update();
}
