#pragma once

#include <string>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "../State/ButtonState.hpp"
#include "../BoardConfig/AbstractBoardConfig.hpp"
#include "AbstractUARTYF.hpp"
#include "MainBoardUARTInterface.hpp"

namespace YFComms {
    class CoverUIControllerUART : public AbstractUARTYF, public CoverUIControllerUARTInterface {

    public:
        CoverUIControllerUART(ButtonState& buttonState, const AbstractBoardConfig& boardConfig,
                                uart_port_t uartPort, gpio_num_t rxPin, gpio_num_t txPin)
            : AbstractUARTYF(uartPort, rxPin, txPin),
              buttonState(buttonState),
              boardConfig(boardConfig)
        {}

        esp_err_t sendYFMessage(const uint8_t* message) override {  // Override, because its ambigously inherited
            return AbstractUARTYF::sendYFMessage(message);  // Delegate to abstract parent
        }

    protected:
        ButtonState& buttonState;
        const AbstractBoardConfig& boardConfig;

        void onUARTReceive(const uint8_t* data, size_t length) override;
        void receiveMessageFromMainboard(const uint8_t* data, size_t length) override;
    };

} // namespace YFComms