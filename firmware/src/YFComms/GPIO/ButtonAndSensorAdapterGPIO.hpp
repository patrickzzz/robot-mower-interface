#pragma once

#include "../State/ButtonState.hpp"
#include "../BoardConfig/AbstractBoardConfig.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "I2CManager.hpp"
#include "esp_io_expander_tca95xx_16bit/include/esp_io_expander_tca95xx_16bit.h"
#include <array>

namespace YFComms {
    class ButtonAndSensorAdapterGPIO {
    public:
        ButtonAndSensorAdapterGPIO(ButtonState& buttonState, const AbstractBoardConfig& boardConfig);

        void start();
        void stop();

    private:
        static void buttonTask(void* pvParameters);
        void updateButtonStates();
        void initGPIOExpander();

        ButtonState& buttonState;
        const AbstractBoardConfig& boardConfig;
        TaskHandle_t buttonTaskHandle;
        esp_io_expander_handle_t ioExpander;

        std::array<uint32_t, static_cast<size_t>(Button::MAX)> lastStateChangeTimes;
    };
} // namespace YFComms