#pragma once

#include <driver/twai.h>

class CanDriver {
    public:
        bool begin(uint8_t tx, uint8_t rx, long baudrate = 1000E3);
        bool sendStandard(uint16_t id, uint8_t data[8], uint8_t dlc);
        bool sendExtended(uint32_t id, uint8_t data[8], uint8_t dlc);
        void onReceive(void (*callback)(twai_message_t msg));

    private:
        twai_timing_config_t twaiTimingConfig(long baudRate);
        static void rxTask(void* param);
        void (*rxCallback)(twai_message_t msg) = nullptr;
};
