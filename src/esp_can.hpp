#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/twai.h"

class CanDriver {
    public:
    bool begin(long baudRate = 1E6, gpio_num_t tx = GPIO_NUM_27, gpio_num_t rx = GPIO_NUM_26);
    bool send(uint32_t id, uint8_t data[8], uint8_t dlc);
    void onReceive(void (*callback)(twai_message_t msg));

    private:
    twai_timing_config_t twaiTimingConfig(long baudRate);
    static void rxTask(void* param);
    void (*rxCallback)(twai_message_t msg) = nullptr;
};


bool CanDriver::begin(long baudRate, gpio_num_t tx, gpio_num_t rx) {
    twai_general_config_t g_config = {
        .mode = TWAI_MODE_NORMAL,   // 通常モード
        .tx_io = tx,
        .rx_io = rx,
        .clkout_io = TWAI_IO_UNUSED,
        .bus_off_io = TWAI_IO_UNUSED,
        .tx_queue_len = 10,
        .rx_queue_len = 10,
        .alerts_enabled = TWAI_ALERT_NONE,
        .clkout_divider = 0,
        .intr_flags = 0
    };

    // 速度設定
    twai_timing_config_t t_config = twaiTimingConfig(baudRate);
    // フィルタ設定（全受信）
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        return false;
    }

    if (twai_start() != ESP_OK) {
        return false;
    }

    xTaskCreate(rxTask, "CAN_RX_Task", 4096, this, 2, NULL);
    return true;
}

void CanDriver::onReceive(void (*callback)(twai_message_t msg)){
    rxCallback = callback;
}

bool CanDriver::send(uint32_t id, uint8_t data[8], uint8_t dlc) {
    twai_message_t msg = {
        .flags = 0,
        .identifier = id,
        .data_length_code = dlc,
    };

    for(int i = 0; i < dlc; i++) {
        msg.data[i] = data[i];
    }

    esp_err_t ret = twai_transmit(&msg, pdMS_TO_TICKS(1000));
    if (ret == ESP_OK) {
      return true;
    } else {
      return false;
    }
}

twai_timing_config_t CanDriver::twaiTimingConfig(long baudRate) {
    switch (baudRate)
    {
    case (long)25E3:
        return TWAI_TIMING_CONFIG_25KBITS();
    case (long)50E3:
        return TWAI_TIMING_CONFIG_50KBITS();
    case (long)100E3:
        return TWAI_TIMING_CONFIG_100KBITS();
    case (long)125E3:
        return TWAI_TIMING_CONFIG_125KBITS();
    case (long)250E3:
        return TWAI_TIMING_CONFIG_250KBITS();
    case (long)500E3:
        return TWAI_TIMING_CONFIG_500KBITS();
    case (long)800E3:
        return TWAI_TIMING_CONFIG_800KBITS();
    case (long)1000E3:
        return TWAI_TIMING_CONFIG_1MBITS();
    default:
        return TWAI_TIMING_CONFIG_1MBITS();
    }
}

void CanDriver::rxTask(void* param) {
    CanDriver* self = static_cast<CanDriver*>(param);
    twai_message_t msg;

    while (true) {
        if (twai_receive(&msg, portMAX_DELAY) == ESP_OK) {
            if (self->rxCallback) {
                self->rxCallback(msg);
            }
        }
    }
}

