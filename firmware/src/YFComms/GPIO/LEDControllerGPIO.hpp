#pragma once

#include "../BoardConfig/AbstractBoardConfig.hpp"
#include <map>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

namespace YFComms {
    class LEDControllerGPIO {
    public:
        LEDControllerGPIO(const AbstractBoardConfig& boardConfig);

        void start();
        void stop();

    private:
        const AbstractBoardConfig& boardConfig;

        TaskHandle_t ledControllerGPIOHandle;

        static void ledControllerGPIOTask(void* pvParameters);
        void updateLEDs(int tickCounter);
        void turnOffAllLEDs();
    };
} // namespace YFComms