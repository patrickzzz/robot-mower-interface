#include "CoverUIController.hpp"
#include <stdexcept>
#include <esp_log.h>

namespace YFComms {

    constexpr char TAG[] = "CoverUIController";

    CoverUIController::CoverUIController(const std::string& modelName)
        : boardConfig(std::make_unique<BoardConfig>(modelName)),
          ledState(),
          buttonState() {}

    void CoverUIController::initialize() {
        ESP_LOGI(TAG, "Initializing CoverUIController for model: %s", boardConfig->getModelName().c_str());
        setupCommunicationHandler();
    }

    void CoverUIController::changeModel(const std::string& modelName) {
        ESP_LOGI(TAG, "Changing model to: %s", modelName.c_str());
        boardConfig = std::make_unique<BoardConfig>(modelName);
        setupCommunicationHandler();
    }

    void CoverUIController::updateButtonStates() {
    /*
        ESP_LOGI(TAG, "Updating button states...");
        for (const auto& buttonConfig : boardConfig->getButtonConfigs()) {
            if (buttonConfig.commType == BoardConfig::CommunicationType::GPIO) {
                buttonState.setState(
                    buttonConfig.buttonIndex,
                    gpio_get_level(static_cast<gpio_num_t>(buttonConfig.gpioPin)) ? ButtonState::State::PRESSED : ButtonState::State::RELEASED
                );
            }
            // Handle other communication types (e.g., GPIO_EXP, UART) here...
        }
     */
    }

    void CoverUIController::updateLEDStates() {
        ESP_LOGI(TAG, "Updating LED states...");
        // Handle LED state updates here...
        commHandler->updateLEDStates(ledState);
    }

    LEDState& CoverUIController::getLEDState() {
        return ledState;
    }

    const ButtonState& CoverUIController::getButtonState() const {
        return buttonState;
    }

    void CoverUIController::setupCommunicationHandler() {
        ESP_LOGI(TAG, "Setting up communication handler...");
        commHandler = std::make_unique<CommunicationHandler>();
        commHandler->initialize(*boardConfig);
    }

} // namespace YFComms