#pragma once

#include "BoardConfig.hpp"
#include "driver/gpio.h"
#include "esp_io_expander.h"
#include "LEDState.hpp"
#include "ButtonState.hpp"

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
                {static_cast<uint8_t>(LED::LIFTED), BoardConfig::CommunicationType::GPIO, GPIO_NUM_1, 0},
                {static_cast<uint8_t>(LED::SIGNAL), BoardConfig::CommunicationType::GPIO, GPIO_NUM_4, 0},
                {static_cast<uint8_t>(LED::BATTERY_LOW), BoardConfig::CommunicationType::GPIO, GPIO_NUM_2, 0},
                {static_cast<uint8_t>(LED::CHARGING), BoardConfig::CommunicationType::GPIO, GPIO_NUM_6, 0},
                {static_cast<uint8_t>(LED::S1), BoardConfig::CommunicationType::GPIO, GPIO_NUM_11, 0},
                {static_cast<uint8_t>(LED::S2), BoardConfig::CommunicationType::GPIO, GPIO_NUM_12, 0},
                {static_cast<uint8_t>(LED::LOCK), BoardConfig::CommunicationType::GPIO, GPIO_NUM_10, 0},
                {static_cast<uint8_t>(LED::HOURS_FOUR), BoardConfig::CommunicationType::GPIO, GPIO_NUM_7, 0},
                {static_cast<uint8_t>(LED::HOURS_SIX), BoardConfig::CommunicationType::GPIO, GPIO_NUM_8, 0},
                {static_cast<uint8_t>(LED::HOURS_EIGHT), BoardConfig::CommunicationType::GPIO, GPIO_NUM_9, 0},
                {static_cast<uint8_t>(LED::HOURS_TEN), BoardConfig::CommunicationType::GPIO, GPIO_NUM_5, 0}
            };

            buttonConfigs = {
                {static_cast<uint8_t>(Button::PLAY), BoardConfig::CommunicationType::GPIO, GPIO_NUM_0, 0xFFFFFFFF},
                {static_cast<uint8_t>(Button::HOME), BoardConfig::CommunicationType::GPIO, GPIO_NUM_3, 0xFFFFFFFF},
                {static_cast<uint8_t>(Button::LOCK), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_12},
                {static_cast<uint8_t>(Button::S1), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_13},
                {static_cast<uint8_t>(Button::S2), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_14},
                {static_cast<uint8_t>(Button::HOURS_FOUR), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_8},
                {static_cast<uint8_t>(Button::HOURS_SIX), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_9},
                {static_cast<uint8_t>(Button::HOURS_EIGHT), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_10},
                {static_cast<uint8_t>(Button::HOURS_TEN), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_11},
            };
        }

        std::vector<uint8_t> getDefaultLEDMessage() const override {
            return {};
        }

        bool hasSerialCommunication() const override {
            return false;
        }
    };
} // namespace YFComms