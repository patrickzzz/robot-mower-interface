#pragma once

#include "BoardConfig.hpp"
#include "ButtonState.hpp"
#include "LEDState.hpp"
#include "CommunicationUART.hpp"

namespace YFComms {

    //class UARTCommunication;  // Platzhalter-Klasse für UART-Kommunikation
    //class GPIOCommunication;  // Platzhalter-Klasse für GPIO-Kommunikation

    class CommunicationHandler {
    public:
        CommunicationHandler();

        void initialize(const BoardConfig& config);

        void updateButtonStates(ButtonState& buttonState);
        void updateLEDStates(const LEDState& ledState);

    private:
        const BoardConfig* boardConfig; // Zeiger auf die aktuelle Board-Konfiguration
        CommunicationUART& uartComm = CommunicationUART::getInstance();
        //std::unique_ptr<UARTCommunication> uartComm;
        //std::unique_ptr<GPIOCommunication> gpioComm;
    };

} // namespace YFComms