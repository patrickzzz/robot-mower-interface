#include "LEDControllerGPIO.hpp"

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "../LED.hpp"
#include "driver/gpio.h"

namespace YFComms {
    LEDControllerGPIO::LEDControllerGPIO(const AbstractBoardConfig& boardConfig)
        : boardConfig(boardConfig), ledControllerGPIOHandle(nullptr) {
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
        for (auto& [name, led] : *boardConfig.leds) {
            if (led.getCommType() != AbstractBoardConfig::CommunicationType::GPIO) // FIXME: Why do we check for CommType::GPIO? We are in the LEDControllerGPIO. Do we have to count with other CommTypes here?
                continue;

            switch (led.getMode()) {
                case led::Modes::ON:
                    led.gpioSetLevel(1);
                    break;

                case led::Modes::OFF:
                    led.gpioSetLevel(0);
                    break;

                case led::Modes::FLASH_SLOW:
                    if (tickCounter % 12 == 0) led.gpioToggle();
                    break;

                case led::Modes::FLASH_FAST:
                    if (tickCounter % 5 == 0) led.gpioToggle();
                    break;

                default:
                    led.gpioSetLevel(0);
                    break;
            }
        }
    }

    // FIXME: A very similar method is already in CoverUIController, but do'nt know how to get it from here.
    //  So I simply build it a second time
    void LEDControllerGPIO::turnOffAllLEDs() {
        for (auto& [name, led] : *boardConfig.leds) {
            if (led.getCommType() != AbstractBoardConfig::CommunicationType::GPIO)  // FIXME: Why do we check for CommType::GPIO? We are in the LEDControllerGPIO. Do we have to count with other CommTypes here?
                continue;
            led.gpioSetLevel(0);
        }
    }
}  // namespace YFComms