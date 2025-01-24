#pragma once

#include "LEDState.hpp"
#include "BoardConfig.hpp"
#include <map>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

namespace YFComms {
    class LEDControllerGPIO {
    public:
        LEDControllerGPIO(LEDState& ledState, const BoardConfig& boardConfig);

        void start();
        void stop();

    private:
        LEDState& ledState;
        const BoardConfig& boardConfig;

        TaskHandle_t ledControllerGPIOHandle;

        std::map<uint8_t, int> ledLastStates;
        static void ledControllerGPIOTask(void* pvParameters);
        void updateLEDs(int tickCounter);
        void toggleLED(const BoardConfig::LEDConfig& ledConfig);
    };
} // namespace YFComms