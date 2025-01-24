#pragma once

#include "BoardConfig.hpp"
#include "driver/gpio.h"
#include "esp_io_expander.h"
#include "LEDState.hpp"

namespace YFComms {
    class BoardConfig500Classic : public BoardConfig {
    public:
        BoardConfig500Classic() : BoardConfig("500Classic") {
            setupConfigs();
        }

        const std::vector<LEDConfig>& getLEDConfigs() const override { return ledConfigs; }
        const std::vector<ButtonConfig>& getButtonConfigs() const override { return buttonConfigs; }

    private:
        std::vector<LEDConfig> ledConfigs;
        std::vector<ButtonConfig> buttonConfigs;

        void setupConfigs() {
            ledConfigs = {
                {static_cast<uint8_t>(LED::LIFTED), BoardConfig::CommunicationType::UART, 0xFF, 0},
                {static_cast<uint8_t>(LED::SIGNAL), BoardConfig::CommunicationType::UART, 0xFF, 1},
                {static_cast<uint8_t>(LED::BATTERY_LOW), BoardConfig::CommunicationType::UART, 0xFF, 2},
                {static_cast<uint8_t>(LED::CHARGING), BoardConfig::CommunicationType::UART, 0xFF, 3},
                {static_cast<uint8_t>(LED::LOCK), BoardConfig::CommunicationType::UART, 0xFF, 10},
                {static_cast<uint8_t>(LED::S1), BoardConfig::CommunicationType::UART, 0xFF, 8},
                {static_cast<uint8_t>(LED::S2), BoardConfig::CommunicationType::UART, 0xFF, 9},
                {static_cast<uint8_t>(LED::HOURS_TWO), BoardConfig::CommunicationType::UART, 0xFF, 4},
                {static_cast<uint8_t>(LED::HOURS_FOUR), BoardConfig::CommunicationType::UART, 0xFF, 5},
                {static_cast<uint8_t>(LED::HOURS_SIX), BoardConfig::CommunicationType::UART, 0xFF, 6},
                {static_cast<uint8_t>(LED::HOURS_EIGHT), BoardConfig::CommunicationType::UART, 0xFF, 7},
                {static_cast<uint8_t>(LED::DAY_MON), BoardConfig::CommunicationType::UART, 0xFF, 11},
                {static_cast<uint8_t>(LED::DAY_TUE), BoardConfig::CommunicationType::UART, 0xFF, 12},
                {static_cast<uint8_t>(LED::DAY_WED), BoardConfig::CommunicationType::UART, 0xFF, 13},
                {static_cast<uint8_t>(LED::DAY_THR), BoardConfig::CommunicationType::UART, 0xFF, 14},
                {static_cast<uint8_t>(LED::DAY_FRI), BoardConfig::CommunicationType::UART, 0xFF, 15},
                {static_cast<uint8_t>(LED::DAY_SAT), BoardConfig::CommunicationType::UART, 0xFF, 16},
                {static_cast<uint8_t>(LED::DAY_SUN), BoardConfig::CommunicationType::UART, 0xFF, 17}
            };

            buttonConfigs = {
                {static_cast<uint8_t>(Button::PLAY), BoardConfig::CommunicationType::GPIO, GPIO_NUM_0, 0xFFFFFFFF},
                {static_cast<uint8_t>(Button::HOME), BoardConfig::CommunicationType::GPIO, GPIO_NUM_3, 0xFFFFFFFF},
                {static_cast<uint8_t>(Button::LOCK), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 3},
            };
        }

        std::vector<uint8_t> getDefaultLEDMessage() const override {
            // here it has an incorrect checksum
            return {0x55, 0xAA, 0x15, 0x50, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        }

        bool hasSerialCommunication() const override {
            return true;
        }
    };
} // namespace YFComms