#pragma once

#include "MainBoardUARTInterface.hpp"
#include "esp_err.h"

namespace YFComms {

    class MainBoardUARTInterface;

    class CoverUIControllerUARTInterface {
    public:
        virtual ~CoverUIControllerUARTInterface() = default;
        virtual esp_err_t sendYFMessage(const uint8_t* message) = 0;
        virtual void receiveMessageFromMainboard(const uint8_t* data, size_t length) = 0;
        void setMainboard(MainBoardUARTInterface* mainboard) {
            this->mainboard = mainboard;
        }

    protected:
        MainBoardUARTInterface* mainboard = nullptr;  // Raw pointer for back-reference
    };

} // namespace YFComms