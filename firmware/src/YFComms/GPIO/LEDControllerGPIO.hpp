#pragma once

#include "../State/LEDState.hpp"
#include "../BoardConfig/AbstractBoardConfig.hpp"
#include <map>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

namespace YFComms {
    class LEDControllerGPIO {
    public:
        LEDControllerGPIO(LEDState& ledState, const AbstractBoardConfig& boardConfig);

        void start();
        void stop();

    private:
        LEDState& ledState;
        const AbstractBoardConfig& boardConfig;

        TaskHandle_t ledControllerGPIOHandle;

        std::map<uint8_t, int> ledLastStates;
        static void ledControllerGPIOTask(void* pvParameters);
        void updateLEDs(int tickCounter);
        void toggleLED(const AbstractBoardConfig::LEDConfig& ledConfig);
        void turnOffAllLEDs();
    };
} // namespace YFComms