#include "CoverUIController.hpp"
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
        if(ledControllerGPIO) {
            ledControllerGPIO->stop();
            ledControllerGPIO.reset();
        }
        if(yfCoverUIControllerUART) {
            yfCoverUIControllerUART->stop();
            yfCoverUIControllerUART.reset();
        }
        if(buttonAndSensorAdapterGPIO) {
            buttonAndSensorAdapterGPIO->stop();
            buttonAndSensorAdapterGPIO.reset();
        }

        boardConfig = BoardConfigFactory(modelName);

        // wait a second to let cover ui serial fade out
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        setupCommunicationHandler();
    }

    LEDState& CoverUIController::getLEDState() {
        return ledState;
    }

    ButtonState& CoverUIController::getButtonState() {
        return buttonState;
    }

    void CoverUIController::setupCommunicationHandler() {
        ESP_LOGI(TAG, "Setting up communication handler...");
        if(boardConfig->hasSerialCommunication()) {
            if(!yfCoverUIControllerUART) {
                yfCoverUIControllerUART = std::make_unique<YFCoverUIControllerUART>(ledState, buttonState, *boardConfig);
            }
            yfCoverUIControllerUART->start();
        }

        if(!ledControllerGPIO) {
            ledControllerGPIO = std::make_unique<LEDControllerGPIO>(ledState, *boardConfig);
        }
        ledControllerGPIO->start();

        if (!buttonAndSensorAdapterGPIO) {
            buttonAndSensorAdapterGPIO = std::make_unique<ButtonAndSensorAdapterGPIO>(buttonState, *boardConfig);
        }
        buttonAndSensorAdapterGPIO->start();
    }
} // namespace YFComms