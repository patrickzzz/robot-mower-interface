#include "ButtonAndSensorAdapterGPIO.hpp"
#include <esp_log.h>

namespace YFComms {
    constexpr char TAG[] = "ButtonAndSensorAdapterGPIO";
    constexpr uint32_t DEBOUNCE_TIME_MS = 10;

    ButtonAndSensorAdapterGPIO::ButtonAndSensorAdapterGPIO(ButtonState& buttonState, const AbstractBoardConfig& boardConfig)
    : buttonState(buttonState), boardConfig(boardConfig), buttonTaskHandle(nullptr) {
        lastStateChangeTimes.fill(0);
    }

    void ButtonAndSensorAdapterGPIO::start() {
        ioExpander = I2CManager::getInstance().getIoExpander();

        if (buttonTaskHandle == nullptr) {
            xTaskCreate(buttonTask, "ButtonTask", 4096, this, 5, &buttonTaskHandle);
        }
    }

    void ButtonAndSensorAdapterGPIO::stop() {
        if (buttonTaskHandle != nullptr) {
            vTaskDelete(buttonTaskHandle);
            buttonTaskHandle = nullptr;
        }
    }

    void ButtonAndSensorAdapterGPIO::buttonTask(void* pvParameters) {
        auto* controller = static_cast<ButtonAndSensorAdapterGPIO*>(pvParameters);
        while (true) {
            controller->updateButtonStates();
            vTaskDelay(10 / portTICK_PERIOD_MS); // Task cycle interval
        }
    }

    void ButtonAndSensorAdapterGPIO::updateButtonStates() {
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

        size_t buttonCount;
        const auto* buttons = boardConfig.getButtonConfigs(buttonCount);

        for (size_t i = 0; i < buttonCount; i++) {
            if (buttons[i].commType == AbstractBoardConfig::CommunicationType::UART) {
                continue; // UART-Buttons read on another place
            }

            // Define state by reading GPIO or Expander
            ButtonStateEnum newState = ButtonStateEnum::LOW;
            if (buttons[i].commType == AbstractBoardConfig::CommunicationType::GPIO_EXP) {
                bool isHigh = (expanderStates & buttons[i].expanderPin) != 0;
                newState = isHigh ? ButtonStateEnum::HIGH : ButtonStateEnum::LOW;
            } else if (buttons[i].commType == AbstractBoardConfig::CommunicationType::GPIO) {
                bool isHigh = gpio_get_level(static_cast<gpio_num_t>(buttons[i].gpioPin)) != 0;
                newState = isHigh ? ButtonStateEnum::HIGH : ButtonStateEnum::LOW;
            }

            // Debounce
            auto index = static_cast<size_t>(buttons[i].buttonIndex);
            uint32_t lastChangeTime = lastStateChangeTimes[index];
            uint32_t duration = currentTime - lastChangeTime;

            if (newState != buttonState.getState(static_cast<Button>(buttons[i].buttonIndex))) {
                if (newState == ButtonStateEnum::LOW && duration < 15) {
                    ESP_LOGI(TAG, "Debounced short press ignored for button %d", buttons[i].buttonIndex);
                    continue; // Ignore short releases
                }

                lastStateChangeTimes[index] = currentTime;
                buttonState.setState(static_cast<Button>(buttons[i].buttonIndex), newState, duration);
            }
        }
    }

} // namespace YFComms