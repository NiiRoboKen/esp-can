#include <Arduino.h>
#include <esp_can.hpp>

CanDriver can;

const gpio_num_t TX_PIN = GPIO_NUM_27;
const gpio_num_t RX_PIN = GPIO_NUM_26;

void canCallback(twai_message_t msg) {
  printf("RX <- ID:0x%lX DLC:%d DATA:", msg.identifier, msg.data_length_code);
  for (int i = 0; i < msg.data_length_code; i++) {
    printf(" %02X", msg.data[i]);
  }
  printf("\n");
}

void setup() {
  if(can.begin(TX_PIN, RX_PIN)) {
    printf("OK\r\n");
  }
  can.onReceive(canCallback);
}

void loop() {
  uint32_t id = 0x00;
  uint8_t data[8] = {1,2,3,4,5,6,7,8};
  can.sendExtended(id, data, sizeof(data));
  delay(1000);
}
