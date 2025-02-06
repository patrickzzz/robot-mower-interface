#include "CoverUIController.hpp"
#include <stdexcept>
#include <esp_log.h>
#include "BoardConfig/BoardConfigFactory.hpp"
#include "driver/uart.h"
#include "driver/gpio.h"

namespace YFComms {
    constexpr char TAG[] = "CoverUIController";

    // Constructor
    CoverUIController::CoverUIController(const std::string& modelName)
    : boardConfig(BoardConfigFactory(modelName)),
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
        if(coverUIControllerUART) {
            coverUIControllerUART.reset();
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

    ButtonState& CoverUIController::getButtonState() {
        return buttonState;
    }

    void CoverUIController::setupCommunicationHandler() {
        ESP_LOGI(TAG, "Setting up communication handler...");
        if(boardConfig->hasSerialCommunication()) {
            if(!coverUIControllerUART) {
                coverUIControllerUART = std::make_unique<CoverUIControllerUART>(buttonState, *boardConfig, uartConfigCoverUI.uartPort, uartConfigCoverUI.rxPin, uartConfigCoverUI.txPin);
            }

            if(coverUIControllerUART->init() != ESP_OK) {
                ESP_LOGE(TAG, "Failed to initialize coverUIController UART");
            }

            if(mainboardType == MainboardType::YF_VIRTUAL) {
                if(!virtualMainboardUART) {
                    virtualMainboardUART = std::make_unique<VirtualMainboardUART>(buttonState, *boardConfig);
                }

                virtualMainboardUART->setCoverUIControllerUART(coverUIControllerUART.get());
                coverUIControllerUART->setMainboard(virtualMainboardUART.get());
            }else if(mainboardType == MainboardType::YF_HARDWARE) {
                if(!hardwareMainboardUART) {
                    hardwareMainboardUART = std::make_unique<HardwareMainboardUART>(buttonState, *boardConfig, uartConfigHardwareMainboard.uartPort, uartConfigHardwareMainboard.rxPin, uartConfigHardwareMainboard.txPin);
                    if(hardwareMainboardUART->init() != ESP_OK) {
                        ESP_LOGE(TAG, "Failed to initialize hardware mainboard UART");
                    }
                }

                hardwareMainboardUART->setCoverUIControllerUART(coverUIControllerUART.get());
                coverUIControllerUART->setMainboard(hardwareMainboardUART.get());
            }
        }else{
            // on serial communication models, no leds are available via gpio
            if(!ledControllerGPIO) {
                ledControllerGPIO = std::make_unique<LEDControllerGPIO>(*boardConfig);
            }
            ledControllerGPIO->start();
        }

        // all models have buttons and sensors available via gpio
        if (!buttonAndSensorAdapterGPIO) {
            buttonAndSensorAdapterGPIO = std::make_unique<ButtonAndSensorAdapterGPIO>(buttonState, *boardConfig);
        }
        buttonAndSensorAdapterGPIO->start();
    }
} // namespace YFComms