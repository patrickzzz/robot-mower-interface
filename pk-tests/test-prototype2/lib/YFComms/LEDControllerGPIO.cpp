#include "LEDControllerGPIO.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

namespace YFComms {
    LEDControllerGPIO::LEDControllerGPIO(LEDState& ledState, const BoardConfig& boardConfig)
        : ledState(ledState), boardConfig(boardConfig), ledControllerGPIOHandle(nullptr) {
        // Initialize all LEDs to off
        for (const auto& ledConfig : boardConfig.getLEDConfigs()) {
            if (ledConfig.commType == BoardConfig::CommunicationType::GPIO) {
                gpio_set_level(static_cast<gpio_num_t>(ledConfig.gpioPin), 0);
                ledLastStates[ledConfig.ledIndex] = 0; // Initialer Zustand der LED (0 = aus)
            }
        }
    }

    void LEDControllerGPIO::start() {
        if (ledControllerGPIOHandle == nullptr) {
            xTaskCreate(ledControllerGPIOTask, "LEDControllerGPIOTask", 4096, this, 5, &ledControllerGPIOHandle);
        }
    }

    void LEDControllerGPIO::stop() {
        if (ledControllerGPIOHandle != nullptr) {
            vTaskDelete(ledControllerGPIOHandle);
            ledControllerGPIOHandle = nullptr;

            // Turn off all LEDs
            for (const auto& ledConfig : boardConfig.getLEDConfigs()) {
                if (ledConfig.commType == BoardConfig::CommunicationType::GPIO) {
                    gpio_set_level(static_cast<gpio_num_t>(ledConfig.gpioPin), 0);
                }
            }
        }
    }

    void LEDControllerGPIO::ledControllerGPIOTask(void* pvParameters) {
        auto* ledControllerGPIO = static_cast<LEDControllerGPIO*>(pvParameters);

        int tickCounter = 0;
        while (true) {
            ledControllerGPIO->updateLEDs(tickCounter);
            tickCounter++;

            // dont let tick counter overflow
            if (tickCounter > 1000) {
                tickCounter = 1;
            }
            vTaskDelay(50 / portTICK_PERIOD_MS); // Aktualisierung alle 50ms
        }
    }

    void LEDControllerGPIO::updateLEDs(int tickCounter) {
        for (const auto& ledConfig : boardConfig.getLEDConfigs()) {
            if (ledConfig.commType == BoardConfig::CommunicationType::GPIO) {
                LEDStateEnum state = ledState.getState(static_cast<LED>(ledConfig.ledIndex));

                switch (state) {
                    case LEDStateEnum::ON:
                        gpio_set_level(static_cast<gpio_num_t>(ledConfig.gpioPin), 1);
                        ledLastStates[ledConfig.ledIndex] = 1;
                        break;

                    case LEDStateEnum::OFF:
                        gpio_set_level(static_cast<gpio_num_t>(ledConfig.gpioPin), 0);
                        ledLastStates[ledConfig.ledIndex] = 0;
                        break;

                    case LEDStateEnum::FLASH_SLOW:
                        if (tickCounter % 12 == 0) {
                            toggleLED(ledConfig);
                        }
                        break;

                    case LEDStateEnum::FLASH_FAST:
                        if (tickCounter % 5 == 0) {
                            toggleLED(ledConfig);
                        }
                        break;

                    default:
                        gpio_set_level(static_cast<gpio_num_t>(ledConfig.gpioPin), 0);
                        ledLastStates[ledConfig.ledIndex] = 0;
                        break;
                }
            }
        }
    }

    void LEDControllerGPIO::toggleLED(const BoardConfig::LEDConfig& ledConfig) {
        int currentLevel = ledLastStates[ledConfig.ledIndex];
        int newLevel = currentLevel == 0 ? 1 : 0;

        gpio_set_level(static_cast<gpio_num_t>(ledConfig.gpioPin), newLevel);
        ledLastStates[ledConfig.ledIndex] = newLevel;

        ESP_LOGI("LEDControllerGPIO", "Toggling LED %d to %d", ledConfig.ledIndex, newLevel);
    }
} // namespace YFComms