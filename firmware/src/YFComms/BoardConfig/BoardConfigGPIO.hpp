#pragma once

#include "AbstractBoardConfig.hpp"
#include "driver/gpio.h"
#include "esp_io_expander.h"
#include "../State/ButtonState.hpp"
#include "../LED.hpp"
#include "../../../include/pins.h"

namespace YFComms {
    class BoardConfigGPIO : public AbstractBoardConfig {
    public:
        BoardConfigGPIO() : AbstractBoardConfig("GPIO") {}

        // With the simple LED class like in YFComms/LED.hpp, we could store all LEDs in such a map,
        // whose key is the LED name which point to the LED object. Pro: We save a lot of space and expensive loops
        etl::flat_map<led::Names, led::LED, 11> leds_impl_ = {
            {led::Names::LIFTED, led::LED(AbstractBoardConfig::CommunicationType::GPIO, pinLedLifted, 0)},
            {led::Names::SIGNAL, led::LED(AbstractBoardConfig::CommunicationType::GPIO, pinLedConnect, 0)},
            {led::Names::BATTERY_LOW, led::LED(AbstractBoardConfig::CommunicationType::GPIO, pinLedBattery, 0)},
            {led::Names::CHARGING, led::LED(AbstractBoardConfig::CommunicationType::GPIO, pinLedCharge, 0)},
            {led::Names::LOCK, led::LED(AbstractBoardConfig::CommunicationType::GPIO, pinLedLock, 0)},  // FIXME: pins.h has another pin than the former one
            {led::Names::S1, led::LED(AbstractBoardConfig::CommunicationType::GPIO, pinLedS1, 0)},      // FIXME: pins.h has another pin than the former one
            {led::Names::S2, led::LED(AbstractBoardConfig::CommunicationType::GPIO, pinLedS2, 0)},      // FIXME: pins.h has another pin than the former one
            {led::Names::HOURS_TWO, led::LED(AbstractBoardConfig::CommunicationType::GPIO, pinLed14h, 0)},
            {led::Names::HOURS_FOUR, led::LED(AbstractBoardConfig::CommunicationType::GPIO, pinLed24h, 0)},
            {led::Names::HOURS_SIX, led::LED(AbstractBoardConfig::CommunicationType::GPIO, pinLed34h, 0)},
            {led::Names::HOURS_EIGHT, led::LED(AbstractBoardConfig::CommunicationType::GPIO, pinLed44h, 0)}};

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