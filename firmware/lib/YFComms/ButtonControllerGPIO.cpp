#include "ButtonControllerGPIO.hpp"
#include <esp_log.h>

namespace YFComms {
    constexpr char TAG[] = "ButtonControllerGPIO";
    constexpr uint32_t DEBOUNCE_TIME_MS = 10;

    ButtonControllerGPIO::ButtonControllerGPIO(ButtonState& buttonState, const BoardConfig& boardConfig)
    : buttonState(buttonState), boardConfig(boardConfig), buttonTaskHandle(nullptr) {
        lastStateChangeTimes.fill(0);
    }

    void ButtonControllerGPIO::start() {
        ioExpander = I2CManager::getInstance().getIoExpander();

        if (buttonTaskHandle == nullptr) {
            xTaskCreate(buttonTask, "ButtonTask", 4096, this, 5, &buttonTaskHandle);
        }
    }

    void ButtonControllerGPIO::stop() {
        if (buttonTaskHandle != nullptr) {
            vTaskDelete(buttonTaskHandle);
            buttonTaskHandle = nullptr;
        }
    }

    void ButtonControllerGPIO::buttonTask(void* pvParameters) {
        auto* controller = static_cast<ButtonControllerGPIO*>(pvParameters);
        while (true) {
            controller->updateButtonStates();
            vTaskDelay(10 / portTICK_PERIOD_MS); // Task cycle interval
        }
    }

    void ButtonControllerGPIO::updateButtonStates() {
        uint32_t expanderStates = 0;
            esp_err_t ret = esp_io_expander_get_level(ioExpander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 | IO_EXPANDER_PIN_NUM_2 |
            IO_EXPANDER_PIN_NUM_3 | IO_EXPANDER_PIN_NUM_4 | IO_EXPANDER_PIN_NUM_5 |
            IO_EXPANDER_PIN_NUM_6 | IO_EXPANDER_PIN_NUM_7 | IO_EXPANDER_PIN_NUM_8 |
            IO_EXPANDER_PIN_NUM_9 | IO_EXPANDER_PIN_NUM_10 | IO_EXPANDER_PIN_NUM_11 |
            IO_EXPANDER_PIN_NUM_12 | IO_EXPANDER_PIN_NUM_13 | IO_EXPANDER_PIN_NUM_14, &expanderStates);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to read Expander states: %s", esp_err_to_name(ret));
            return;
        }

        uint32_t currentTime = xTaskGetTickCount() * portTICK_PERIOD_MS;

        for (const auto& buttonConfig : boardConfig.getButtonConfigs()) {
            if (buttonConfig.commType == BoardConfig::CommunicationType::UART) {
                continue; // UART-Buttons werden anders behandelt
            }

            // Button State bestimmen (GPIO oder GPIO_EXP)
            ButtonStateEnum newState = ButtonStateEnum::RELEASED;
            if (buttonConfig.commType == BoardConfig::CommunicationType::GPIO_EXP) {
                bool isPressed = (expanderStates & buttonConfig.expanderPin) == 0; // Active Low
                newState = isPressed ? ButtonStateEnum::PRESSED : ButtonStateEnum::RELEASED;
            } else if (buttonConfig.commType == BoardConfig::CommunicationType::GPIO) {
                int level = gpio_get_level(static_cast<gpio_num_t>(buttonConfig.gpioPin));
                newState = (level == 0) ? ButtonStateEnum::PRESSED : ButtonStateEnum::RELEASED;
            }

            // Zeitmessung und Debounce
            auto index = static_cast<size_t>(buttonConfig.buttonIndex);
            uint32_t lastChangeTime = lastStateChangeTimes[index];
            uint32_t duration = currentTime - lastChangeTime;

            if (newState != buttonState.getState(static_cast<Button>(buttonConfig.buttonIndex))) {
                if (newState == ButtonStateEnum::RELEASED && duration < 15) {
                    ESP_LOGI(TAG, "Debounced short press ignored for button %d", buttonConfig.buttonIndex);
                    continue; // Ignore short releases
                }

                // Update Zeitstempel und Zustand
                lastStateChangeTimes[index] = currentTime;
                buttonState.setState(static_cast<Button>(buttonConfig.buttonIndex), newState, duration);
            }
        }
    }

} // namespace YFComms