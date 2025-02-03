#pragma once

#include "driver/uart.h"
#include <cstddef>

namespace YFComms {
    class AbstractUARTEventListener {
    public:
        virtual ~AbstractUARTEventListener() = default;
        virtual void onMsgReceiveEvent(
            const uint8_t* message,
            size_t length,
            uart_port_t eventUartNum,
            int eventTxIoNum,
            int eventRxIoNum
        ) = 0;
    };
} // namespace YFComms