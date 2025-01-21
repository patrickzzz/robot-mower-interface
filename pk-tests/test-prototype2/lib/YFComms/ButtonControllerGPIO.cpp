#include "ButtonControllerGPIO.hpp"
#include <esp_log.h>

namespace YFComms {
    constexpr char TAG[] = "ButtonControllerGPIO";

    ButtonControllerGPIO::ButtonControllerGPIO(ButtonState& buttonState, const BoardConfig& boardConfig)
        : buttonState(buttonState), boardConfig(boardConfig), buttonTaskHandle(nullptr) {}

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
            vTaskDelay(10 / portTICK_PERIOD_MS); // 10ms debounce interval
        }
    }

    void ButtonControllerGPIO::updateButtonStates() {
        uint32_t expanderStates = 0;
        // Read Expander states

        esp_err_t ret = esp_io_expander_get_level(ioExpander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 | IO_EXPANDER_PIN_NUM_2 |
            IO_EXPANDER_PIN_NUM_3 | IO_EXPANDER_PIN_NUM_4 | IO_EXPANDER_PIN_NUM_5 |
            IO_EXPANDER_PIN_NUM_6 | IO_EXPANDER_PIN_NUM_7 | IO_EXPANDER_PIN_NUM_8 |
            IO_EXPANDER_PIN_NUM_9 | IO_EXPANDER_PIN_NUM_10 | IO_EXPANDER_PIN_NUM_11 |
            IO_EXPANDER_PIN_NUM_12 | IO_EXPANDER_PIN_NUM_13 | IO_EXPANDER_PIN_NUM_14, &expanderStates);

        //esp_err_t ret = esp_io_expander_get_level(ioExpander, IO_EXPANDER_PIN_NUM_12, &expanderStates);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to read Expander states: %s", esp_err_to_name(ret));
            return;
        }

        //ESP_LOGI(TAG, "Expander States: 0x%04X", static_cast<unsigned int>(expanderStates));

        // Process each button
        for (const auto& buttonConfig : boardConfig.getButtonConfigs()) {
            ButtonStateEnum state = ButtonStateEnum::RELEASED;

            if (buttonConfig.commType == BoardConfig::CommunicationType::GPIO_EXP) {
                //ESP_LOGI(TAG, "Checking button %d with expanderPin %d", buttonConfig.buttonIndex, static_cast<int>(buttonConfig.expanderPin));
                bool isPressed = (expanderStates & buttonConfig.expanderPin) == 0; // Active low
                //ESP_LOGI(TAG, "Button %d: isPressed=%s", buttonConfig.buttonIndex, isPressed ? "true" : "false");
                state = isPressed ? ButtonStateEnum::PRESSED : ButtonStateEnum::RELEASED;
            } else if (buttonConfig.commType == BoardConfig::CommunicationType::GPIO) {
                int level = gpio_get_level(static_cast<gpio_num_t>(buttonConfig.gpioPin));
                state = (level == 0) ? ButtonStateEnum::PRESSED : ButtonStateEnum::RELEASED;
            }

            buttonState.setState(static_cast<Button>(buttonConfig.buttonIndex), state);

            if (state == ButtonStateEnum::PRESSED) {
                //ESP_LOGI(TAG, "Button %d pressed", buttonConfig.buttonIndex);
                // log button name instead

            }else{
            //    ESP_LOGI(TAG, "Button %d released", buttonConfig.buttonIndex);
            }
        }
    }

} // namespace YFComms