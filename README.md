# esp-can

ESP32の内蔵CANコントローラ（TWAI: Two-Wire Automotive Interface）を、Arduino環境で簡単に扱うためのラッパークラスライブラリです。
ESP-IDFの `driver/twai` をベースにしており、FreeRTOSタスクを用いた非同期受信をサポートしています。

## 特徴

- **シンプルなAPI**: `begin`, `send`, `onReceive` などの直感的なメソッドで操作可能。
- **標準/拡張フレーム対応**: Standard ID (11-bit) と Extended ID (29-bit) の両方をサポート。
- **非同期受信**: 受信処理はバックグラウンドのFreeRTOSタスクで行われるため、メインループ (`loop()`) をブロックしません。
- **コールバック通知**: データ受信時に登録した関数を自動的に呼び出します。

## PlatformIOでの設定

PlatformIOを使用する場合は、`platformio.ini` に以下のように記述します。`lib_deps` にこのライブラリのGitリポジトリのURL（またはローカルパス）を追加することで、自動的にインストールして使用できます。

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    ; GitHub等のリポジトリURLをここに記述
    https://github.com/NiiRoboKen/esp-can.git
```

## 対応ボーレート

begin 関数で以下の速度（long型）を指定可能です。

- 25,000 (25kbps)
- 50,000 (50kbps)
- 100,000 (100kbps)
- 125,000 (125kbps)
- 250,000 (250kbps)
- 500,000 (500kbps)
- 800,000 (800kbps)
- 1,000,000 (1Mbps)

## API リファレンス

### 初期化
```cpp
bool begin(long baudRate, uint8_t tx, uint8_t rx);
```

ドライバを初期化し、CANバス通信を開始します。

- `baudRate`: 通信速度 (例: 500E3, 1000E3)
- `tx`: TXピン番号 (GPIO)
- `rx`: RXピン番号 (GPIO)
- **戻り値**: 成功時は `true`、失敗時は `false`

### 送信

#### 標準フレーム (Standard ID)
```cpp
bool sendStandard(uint16_t id, uint8_t data[8], uint8_t dlc);
```

#### 拡張フレーム (Extended ID)
```cpp
bool sendExtended(uint32_t id, uint8_t data[8], uint8_t dlc);
```

- `id`: CAN ID
- `data`: 送信データ配列（最大8バイト）
- `dlc`: データ長 (0-8)
- **戻り値**: 送信キューへの追加が成功すれば `true`

### 受信コールバック登録
```cpp
void onReceive(void (*callback)(twai_message_t msg));
```

メッセージを受信した際に呼び出される関数を登録します。

- `callback`: `twai_message_t` 型を受け取る関数ポインタ

### 使用例 (Example)
```cpp
#include <Arduino.h>
#include <esp_can.hpp>

CanDriver can;

// ピン設定 (使用するボードに合わせて変更してください)
const gpio_num_t TX_PIN = GPIO_NUM_27;
const gpio_num_t RX_PIN = GPIO_NUM_26;

// 受信時のコールバック関数
void canCallback(twai_message_t msg) {
  printf("RX <- ID:0x%lX DLC:%d DATA:", msg.identifier, msg.data_length_code);
  for (int i = 0; i < msg.data_length_code; i++) {
    printf(" %02X", msg.data[i]);
  }
  printf("\n");
}

void setup() {
  Serial.begin(115200);
  
  // 1Mbpsで初期化
  if(can.begin(1000E3, TX_PIN, RX_PIN)) {
    printf("CAN Init OK\r\n");
  } else {
    printf("CAN Init Failed\r\n");
  }

  // コールバックの登録
  can.onReceive(canCallback);
}

void loop() {
  // 1秒ごとに拡張フレームを送信する例
  uint32_t id = 0x12345;
  uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  
  if(can.sendExtended(id, data, 8)) {
      Serial.println("Message Sent");
  }
  
  delay(1000);
}
```

### 注意事項

- **Transceiver**: 別途CANトランシーバー（MCP25625等）が必要です。ESP32のピンを直接CANバスに接続しないでください。
- **Termination**: バスの両端には120Ωの終端抵抗が必要です。
- **Queue Size**: 送受信キューのサイズはコード内で `10` に固定されています。変更が必要な場合は `CanDriver::begin` 内の `twai_general_config_t` を編集してください。