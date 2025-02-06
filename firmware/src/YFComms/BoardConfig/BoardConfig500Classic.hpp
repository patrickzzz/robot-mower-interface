#pragma once

#include <etl/flat_map.h>

#include "../LED.hpp"
#include "AbstractBoardConfig.hpp"
#include "driver/gpio.h"
#include "esp_io_expander.h"

namespace YFComms {
class BoardConfig500Classic : public AbstractBoardConfig {
   public:
    BoardConfig500Classic() : AbstractBoardConfig("500Classic") {
        leds = &leds_impl_; // Assign our private LED declaration to base class leds pointer
    }

    // With the simple LED class like in YFComms/LED.hpp, we could store all LEDs in such a map,
    // whose key is the LED name which point to the LED object. Pro: We save a lot of space and expensive loops
    etl::flat_map<led::Names, led::LED, 18> leds_impl_ = {
        {led::Names::LIFTED, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 0)},
        {led::Names::SIGNAL, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 1)},
        {led::Names::BATTERY_LOW, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 2)},
        {led::Names::CHARGING, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 3)},
        {led::Names::LOCK, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 10)},
        {led::Names::S1, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 8)},
        {led::Names::S2, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 9)},
        {led::Names::HOURS_TWO, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 4)},
        {led::Names::HOURS_FOUR, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 5)},
        {led::Names::HOURS_SIX, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 6)},
        {led::Names::HOURS_EIGHT, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 7)},
        {led::Names::DAY_MON, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 11)},
        {led::Names::DAY_TUE, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 12)},
        {led::Names::DAY_WED, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 13)},
        {led::Names::DAY_THR, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 14)},
        {led::Names::DAY_FRI, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 15)},
        {led::Names::DAY_SAT, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 16)},
        {led::Names::DAY_SUN, led::LED(AbstractBoardConfig::CommunicationType::UART, GPIO_NUM_NC, 17)}
    };

    const ButtonConfig* getButtonConfigs(size_t& count) const override {
        static constexpr ButtonConfig buttonConfigs[] = {
            // GPIO
            {static_cast<uint8_t>(Button::PLAY), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_0, 0xFFFFFFFF, 0xFF},
            {static_cast<uint8_t>(Button::HOME), AbstractBoardConfig::CommunicationType::GPIO, GPIO_NUM_3, 0xFFFFFFFF, 0xFF},

            // UART
            {static_cast<uint8_t>(Button::LOCK), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 3},
            {static_cast<uint8_t>(Button::S1), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 1},
            {static_cast<uint8_t>(Button::S2), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 2},
            {static_cast<uint8_t>(Button::OK), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 4},
            {static_cast<uint8_t>(Button::CLOCK), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 0},
            {static_cast<uint8_t>(Button::DAYS_MON), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 5},
            {static_cast<uint8_t>(Button::DAYS_TUE), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 6},
            {static_cast<uint8_t>(Button::DAYS_WED), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 7},
            {static_cast<uint8_t>(Button::DAYS_THR), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 8},
            {static_cast<uint8_t>(Button::DAYS_FRI), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 9},
            {static_cast<uint8_t>(Button::DAYS_SAT), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 10},
            {static_cast<uint8_t>(Button::DAYS_SUN), AbstractBoardConfig::CommunicationType::UART, 0xFF, 0xFFFFFFFF, 11},

            // GPIO_EXP
            {static_cast<uint8_t>(Button::STOP1), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_3, 0xFF},
            {static_cast<uint8_t>(Button::STOP2), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_0, 0xFF},
            {static_cast<uint8_t>(Button::LIFT), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_5, 0xFF},
            {static_cast<uint8_t>(Button::LIFTX), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_1, 0xFF},
            {static_cast<uint8_t>(Button::BUMPL), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_2, 0xFF},
            {static_cast<uint8_t>(Button::BUMPR), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_4, 0xFF},
            // GPIO_EXP SHELL STOP
            {static_cast<uint8_t>(Button::SHELL_STOP1), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_6, 0xFF},
            {static_cast<uint8_t>(Button::SHELL_STOP2), AbstractBoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_7, 0xFF},
        };
        count = sizeof(buttonConfigs) / sizeof(buttonConfigs[0]);  // Größe berechnen
        return buttonConfigs;
    }

    const uint8_t* getDefaultLEDMessage(size_t& length) const override {
        static constexpr uint8_t message[] = {
            0x55, 0xAA, 0x15, 0x50, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00};
        length = sizeof(message);
        return message;
    }

    bool hasSerialCommunication() const override {
        return true;
    }
};
}  // namespace YFComms