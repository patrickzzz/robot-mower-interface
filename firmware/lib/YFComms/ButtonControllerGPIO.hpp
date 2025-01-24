#pragma once

#include "ButtonState.hpp"
#include "BoardConfig.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "I2CManager.hpp"
#include "esp_io_expander_tca95xx_16bit/include/esp_io_expander_tca95xx_16bit.h"

namespace YFComms {
    class ButtonControllerGPIO {
    public:
        ButtonControllerGPIO(ButtonState& buttonState, const BoardConfig& boardConfig);

        void start();
        void stop();

    private:
        static void buttonTask(void* pvParameters);
        void updateButtonStates();
        void initGPIOExpander();

        ButtonState& buttonState;
        const BoardConfig& boardConfig;
        TaskHandle_t buttonTaskHandle;
        esp_io_expander_handle_t ioExpander;
    };
} // namespace YFComms