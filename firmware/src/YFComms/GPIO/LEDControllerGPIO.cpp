#include "LEDControllerGPIO.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

namespace YFComms {
    LEDControllerGPIO::LEDControllerGPIO(LEDState& ledState, const AbstractBoardConfig& boardConfig)
        : ledState(ledState), boardConfig(boardConfig), ledControllerGPIOHandle(nullptr) {
        // Initialize all LEDs to off
        turnOffAllLEDs();
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
            turnOffAllLEDs();
        }
    }

    void LEDControllerGPIO::ledControllerGPIOTask(void* pvParameters) {
        ESP_LOGI("LEDControllerGPIO", "Starting LEDControllerGPIOTask");
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
        size_t ledCount;
        const auto* leds = boardConfig.getLEDConfigs(ledCount);

        for (size_t i = 0; i < ledCount; i++) {
            if (leds[i].commType == AbstractBoardConfig::CommunicationType::GPIO) {
                LEDStateEnum state = ledState.getState(static_cast<LED>(leds[i].ledIndex));

                switch (state) {
                    case LEDStateEnum::ON:
                        gpio_set_level(static_cast<gpio_num_t>(leds[i].gpioPin), 1);
                        ledLastStates[leds[i].ledIndex] = 1;
                        break;

                    case LEDStateEnum::OFF:
                        gpio_set_level(static_cast<gpio_num_t>(leds[i].gpioPin), 0);
                        ledLastStates[leds[i].ledIndex] = 0;
                        break;

                    case LEDStateEnum::FLASH_SLOW:
                        if (tickCounter % 12 == 0) {
                            toggleLED(leds[i]);
                        }
                        break;

                    case LEDStateEnum::FLASH_FAST:
                        if (tickCounter % 5 == 0) {
                            toggleLED(leds[i]);
                        }
                        break;

                    default:
                        gpio_set_level(static_cast<gpio_num_t>(leds[i].gpioPin), 0);
                        ledLastStates[leds[i].ledIndex] = 0;
                        break;
                }
            }
        }
    }

    void LEDControllerGPIO::toggleLED(const AbstractBoardConfig::LEDConfig& ledConfig) {
        int currentLevel = ledLastStates[ledConfig.ledIndex];
        int newLevel = currentLevel == 0 ? 1 : 0;

        gpio_set_level(static_cast<gpio_num_t>(ledConfig.gpioPin), newLevel);
        ledLastStates[ledConfig.ledIndex] = newLevel;
    }

    void LEDControllerGPIO::turnOffAllLEDs() {
        size_t ledCount;
        const auto* leds = boardConfig.getLEDConfigs(ledCount);

        for (size_t i = 0; i < ledCount; i++) {
            if (leds[i].commType == AbstractBoardConfig::CommunicationType::GPIO) {
                gpio_set_level(static_cast<gpio_num_t>(leds[i].gpioPin), 0);
                ledLastStates[leds[i].ledIndex] = 0;
            }
        }
    }
} // namespace YFComms