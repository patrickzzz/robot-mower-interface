#pragma once

#include "AbstractBoardConfig.hpp"
#include "driver/gpio.h"
#include "esp_io_expander.h"
#include "../State/LEDState.hpp"
#include "../State/ButtonState.hpp"

namespace YFComms {
    class BoardConfigGPIO : public AbstractBoardConfig {
    public:
        BoardConfigGPIO() : AbstractBoardConfig("GPIO") {}

        const LEDConfig* getLEDConfigs(size_t& count) const override {
            static constexpr LEDConfig ledConfigs[] = {
                {static_cast<uint8_t>(LED::LIFTED), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_1, 0},
                {static_cast<uint8_t>(LED::SIGNAL), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_4, 0},
                {static_cast<uint8_t>(LED::BATTERY_LOW), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_2, 0},
                {static_cast<uint8_t>(LED::CHARGING), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_6, 0},
                {static_cast<uint8_t>(LED::S1), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_11, 0},
                {static_cast<uint8_t>(LED::S2), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_12, 0},
                {static_cast<uint8_t>(LED::LOCK), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_10, 0},
                {static_cast<uint8_t>(LED::HOURS_FOUR), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_7, 0},
                {static_cast<uint8_t>(LED::HOURS_SIX), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_8, 0},
                {static_cast<uint8_t>(LED::HOURS_EIGHT), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_9, 0},
                {static_cast<uint8_t>(LED::HOURS_TEN), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_5, 0}
            };

            count = sizeof(ledConfigs) / sizeof(ledConfigs[0]);

            return ledConfigs;
        }

        const ButtonConfig* getButtonConfigs(size_t& count) const override {
            static constexpr ButtonConfig buttonConfigs[] = {
                {static_cast<uint8_t>(Button::PLAY), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_0, 0xFFFFFFFF, 0xFF},
                {static_cast<uint8_t>(Button::HOME), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_3, 0xFFFFFFFF, 0xFF},
                {static_cast<uint8_t>(Button::LOCK), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_12, 0xFF},
                {static_cast<uint8_t>(Button::S1), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_13, 0xFF},
                {static_cast<uint8_t>(Button::S2), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_14, 0xFF},
                {static_cast<uint8_t>(Button::HOURS_FOUR), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_8, 0xFF},
                {static_cast<uint8_t>(Button::HOURS_SIX), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_9, 0xFF},
                {static_cast<uint8_t>(Button::HOURS_EIGHT), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_10, 0xFF},
                {static_cast<uint8_t>(Button::HOURS_TEN), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_11, 0xFF},
            };
            count = sizeof(buttonConfigs) / sizeof(buttonConfigs[0]);  // Größe berechnen
            return buttonConfigs;
        }

        bool hasSerialCommunication() const override {
            return false;
        }

        const uint8_t* getDefaultLEDMessage(size_t& length) const override {
            length = 0;
            return nullptr;
        }
    };
} // namespace YFComms