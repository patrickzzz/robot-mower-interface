#pragma once

#include "BoardConfig.hpp"
#include "driver/gpio.h"
#include "esp_io_expander.h"
#include "LEDState.hpp"

namespace YFComms {
    class BoardConfig500Classic : public BoardConfig {
    public:
        BoardConfig500Classic() : BoardConfig("500Classic") {}

        const LEDConfig* getLEDConfigs(size_t& count) const override {
            static constexpr LEDConfig ledConfigs[] = {
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

            count = sizeof(ledConfigs) / sizeof(ledConfigs[0]);

            return ledConfigs;
        }

        const ButtonConfig* getButtonConfigs(size_t& count) const override {
            static constexpr ButtonConfig buttonConfigs[] = {
                // GPIO
                {static_cast<uint8_t>(Button::PLAY), BoardConfig::CommunicationType::GPIO, GPIO_NUM_0, 0xFFFFFFFF, 0xFF},
                {static_cast<uint8_t>(Button::HOME), BoardConfig::CommunicationType::GPIO, GPIO_NUM_3, 0xFFFFFFFF, 0xFF},

                // UART
                {static_cast<uint8_t>(Button::LOCK), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 3},
                {static_cast<uint8_t>(Button::S1), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 1},
                {static_cast<uint8_t>(Button::S2), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 2},
                {static_cast<uint8_t>(Button::OK), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 4},
                {static_cast<uint8_t>(Button::CLOCK), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 0},
                {static_cast<uint8_t>(Button::DAYS_MON), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 5},
                {static_cast<uint8_t>(Button::DAYS_TUE), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 6},
                {static_cast<uint8_t>(Button::DAYS_WED), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 7},
                {static_cast<uint8_t>(Button::DAYS_THR), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 8},
                {static_cast<uint8_t>(Button::DAYS_FRI), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 9},
                {static_cast<uint8_t>(Button::DAYS_SAT), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 10},
                {static_cast<uint8_t>(Button::DAYS_SUN), BoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 11},

                // GPIO_EXP
                {static_cast<uint8_t>(Button::STOP1), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_3, 0xFF},
                {static_cast<uint8_t>(Button::STOP2), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_0, 0xFF},
                {static_cast<uint8_t>(Button::LIFT), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_5, 0xFF},
                {static_cast<uint8_t>(Button::LIFTX), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_1, 0xFF},
                {static_cast<uint8_t>(Button::BUMPL), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_2, 0xFF},
                {static_cast<uint8_t>(Button::BUMPR), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_4, 0xFF},
                // GPIO_EXP SHELL STOP
                {static_cast<uint8_t>(Button::SHELL_STOP1), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_6, 0xFF},
                {static_cast<uint8_t>(Button::SHELL_STOP2), BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_7, 0xFF},
            };
            count = sizeof(buttonConfigs) / sizeof(buttonConfigs[0]);  // Größe berechnen
            return buttonConfigs;
        }

        const uint8_t* getDefaultLEDMessage(size_t& length) const override {
            static constexpr uint8_t message[] = {
                0x55, 0xAA, 0x15, 0x50, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00
            };
            length = sizeof(message);
            return message;
        }

        bool hasSerialCommunication() const override {
            return true;
        }
    };
} // namespace YFComms