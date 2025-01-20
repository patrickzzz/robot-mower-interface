#pragma once

#include "BoardConfig.hpp"
#include "driver/gpio.h"
#include "esp_io_expander.h"
#include "LEDState.hpp"

namespace YFComms {
    class BoardConfigGPIO : public BoardConfig {
    public:
        BoardConfigGPIO() : BoardConfig("GPIO") {
            setupConfigs();
        }

        const std::vector<LEDConfig>& getLEDConfigs() const override { return ledConfigs; }
        const std::vector<ButtonConfig>& getButtonConfigs() const override { return buttonConfigs; }

    private:
        std::vector<LEDConfig> ledConfigs;
        std::vector<ButtonConfig> buttonConfigs;

        void setupConfigs() {
            ledConfigs = {
                {static_cast<uint8_t>(LED::LIFTED), BoardConfig::CommunicationType::GPIO, GPIO_NUM_1, 0},    // Lifted
                {static_cast<uint8_t>(LED::SIGNAL), BoardConfig::CommunicationType::GPIO, GPIO_NUM_2, 0},    // Connect (WLAN Symbol)
                {static_cast<uint8_t>(LED::BATTERY_LOW), BoardConfig::CommunicationType::GPIO, GPIO_NUM_3, 0},
                {static_cast<uint8_t>(LED::CHARGING), BoardConfig::CommunicationType::GPIO, GPIO_NUM_4, 0},
                {static_cast<uint8_t>(LED::S1), BoardConfig::CommunicationType::GPIO, GPIO_NUM_27, 0},
                {static_cast<uint8_t>(LED::S2), BoardConfig::CommunicationType::GPIO, GPIO_NUM_28, 0},
                {static_cast<uint8_t>(LED::LOCK), BoardConfig::CommunicationType::GPIO, GPIO_NUM_29, 0},
                {static_cast<uint8_t>(LED::HOURS_FOUR), BoardConfig::CommunicationType::GPIO, GPIO_NUM_10, 0},
                {static_cast<uint8_t>(LED::HOURS_SIX), BoardConfig::CommunicationType::GPIO, GPIO_NUM_11, 0},
                {static_cast<uint8_t>(LED::HOURS_EIGHT), BoardConfig::CommunicationType::GPIO, GPIO_NUM_12, 0},
                {static_cast<uint8_t>(LED::HOURS_TEN), BoardConfig::CommunicationType::GPIO, GPIO_NUM_13, 0}
            };

            buttonConfigs = {
                {0, BoardConfig::CommunicationType::GPIO, GPIO_NUM_0, 0xFFFFFFFF}, // Play Button
                {1, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_6},  // Home Button
                {10, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_8}, // Lock
                {11, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_9}, // S1
                {12, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_10}, // S2
                {13, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_11}, // 4 hr
                {14, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_12}, // 6 hr
                {15, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_13}, // 8 hr
                {16, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_14}, // 10 hr
            };
        }
    };
} // namespace YFComms