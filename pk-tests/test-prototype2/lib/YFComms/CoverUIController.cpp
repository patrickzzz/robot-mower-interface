#include "CoverUIController.hpp"
#include "BoardConfig.hpp"
#include "BoardConfig/BoardConfigFactory.hpp"
#include <stdexcept>
#include <esp_log.h>


namespace YFComms {
    constexpr char TAG[] = "CoverUIController";

    CoverUIController::CoverUIController(const std::string& modelName)
        : boardConfig(BoardConfigFactory(modelName)),
          ledState(),
          buttonState() {}

    void CoverUIController::initialize() {
        ESP_LOGI(TAG, "Initializing CoverUIController for model: %s", boardConfig->getModelName().c_str());
        setupCommunicationHandler();
    }

    void CoverUIController::changeModel(const std::string& modelName) {
        ESP_LOGI(TAG, "Changing model to: %s", modelName.c_str());
        ledControllerGPIO->stop();
        ledControllerGPIO.reset();
        yfCoverUIControllerUART->stop();
        yfCoverUIControllerUART.reset();

        boardConfig = BoardConfigFactory(modelName);

        // wait a second to let cover ui serial fade out
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        setupCommunicationHandler();
    }

    void CoverUIController::updateButtonStates() {
    }

    void CoverUIController::updateLEDStates() {
        ESP_LOGI(TAG, "Updating LED states...");
        for (const auto& ledConfig : boardConfig->getLEDConfigs()) {
            if (ledConfig.commType == BoardConfig::CommunicationType::GPIO) {
                // GPIO-LED setzen
                //gpio_set_level(static_cast<gpio_num_t>(ledConfig.gpioPin), static_cast<uint8_t>(ledState.getState(static_cast<LED>(ledConfig.ledIndex))) > 0);
            } else if (ledConfig.commType == BoardConfig::CommunicationType::UART) {
                yfCoverUIControllerUART->setLEDStateInMessage(ledConfig.uartMessagePos, ledState.getState(static_cast<LED>(ledConfig.ledIndex)));
            }
        }
    }

    LEDState& CoverUIController::getLEDState() {
        return ledState;
    }

    const ButtonState& CoverUIController::getButtonState() const {
        return buttonState;
    }

    void CoverUIController::setupCommunicationHandler() {
        ESP_LOGI(TAG, "Setting up communication handler...");
        if(!yfCoverUIControllerUART)
        {
        yfCoverUIControllerUART = std::make_unique<YFCoverUIControllerUART>(ledState, *boardConfig);
        }
        yfCoverUIControllerUART->start();

        if(!ledControllerGPIO) {
        ledControllerGPIO = std::make_unique<LEDControllerGPIO>(ledState, *boardConfig);
        }
        ledControllerGPIO->start();
    }
} // namespace YFComms