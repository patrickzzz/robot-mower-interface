#include "CommunicationHandler.hpp"
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace YFComms {
    constexpr char TAG[] = "CommunicationHandler";

    CommunicationHandler::CommunicationHandler() 
    //    : boardConfig(nullptr), uartComm(nullptr), gpioComm(nullptr) {}
    : boardConfig(nullptr) {}

    void CommunicationHandler::initialize(const BoardConfig& config) {
        ESP_LOGI(TAG, "Initializing CommunicationHandler...");
        boardConfig = &config;


//        TaskHandle_t yf_comms_task_handle = NULL;
//        YFComms::CommunicationUART communicationUART;
//        xTaskCreate(YFComms::CommunicationUART::task, "YFComms_Task", 4096, &communicationUART, 10, &yf_comms_task_handle);
        //auto& uartComm = CommunicationUART::getInstance();
        //uartComm.initializeUart();
        //uartComm.tryHandshake();

        // Optional: Task starten
        xTaskCreate(CommunicationUART::task, "YFComms_Task", 4096, nullptr, 2, nullptr);

        // Initialisiere die UART- und GPIO-Kommunikation
        //uartComm = std::make_unique<UARTCommunication>();
        //gpioComm = std::make_unique<GPIOCommunication>();

        // Beispielhafte Initialisierungen (in echten Implementierungen spezifisch)
        // uartComm->initialize();
        // gpioComm->initialize();
    }

    void CommunicationHandler::updateButtonStates(ButtonState& buttonState) {
        ESP_LOGI(TAG, "Updating button states...");
        for (const auto& buttonConfig : boardConfig->getButtonConfigs()) {
            if (buttonConfig.commType == BoardConfig::CommunicationType::GPIO) {
                // ...
            } else if (buttonConfig.commType == BoardConfig::CommunicationType::GPIO_EXP) {
                // ...
            } else if (buttonConfig.commType == BoardConfig::CommunicationType::UART) {
                // ...
            }
        }
    }

    void CommunicationHandler::updateLEDStates(const LEDState& ledState) {
        ESP_LOGI(TAG, "Updating LED states...");
        for (const auto& ledConfig : boardConfig->getLEDConfigs()) {
            if (ledConfig.commType == BoardConfig::CommunicationType::GPIO) {
                // GPIO-LED setzen
                gpio_set_level(static_cast<gpio_num_t>(ledConfig.gpioPin), static_cast<uint8_t>(ledState.getState(static_cast<LED>(ledConfig.ledIndex))) > 0);
            } else if (ledConfig.commType == BoardConfig::CommunicationType::UART) {
                uartComm.setLEDStateInMessage(ledConfig.uartMessagePos, ledState.getState(static_cast<LED>(ledConfig.ledIndex)));
            }
        }
    }

} // namespace YFComms