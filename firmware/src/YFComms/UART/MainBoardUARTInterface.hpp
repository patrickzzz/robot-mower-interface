#pragma once

#include <string>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "CoverUIControllerUARTInterface.hpp"

namespace YFComms {

    class MainBoardUARTInterface {
    public:
        virtual ~MainBoardUARTInterface() = default;

        virtual void receiveMessageFromCoverUI(const uint8_t* data, size_t length) = 0;
        void setCoverUIControllerUART(CoverUIControllerUARTInterface* coverUIControllerUART) {
            this->coverUIControllerUART = coverUIControllerUART;
        }

    protected:
        CoverUIControllerUARTInterface* coverUIControllerUART;
    };

} // namespace YFComms
