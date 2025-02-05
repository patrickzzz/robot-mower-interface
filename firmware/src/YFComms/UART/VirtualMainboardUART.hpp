#pragma once

#include <string>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "../State/LEDState.hpp"
#include "../State/ButtonState.hpp"
#include "../BoardConfig/AbstractBoardConfig.hpp"
#include "AbstractUARTYF.hpp"
#include "MainBoardUARTInterface.hpp"

namespace YFComms {
    class VirtualMainboardUART : public AbstractUARTYF, public MainBoardUARTInterface {

    public:
        VirtualMainboardUART(LEDState& ledState, ButtonState& buttonState, const AbstractBoardConfig& boardConfig)
            : AbstractUARTYF(UART_NUM_MAX, GPIO_NUM_NC, GPIO_NUM_NC),
              ledState(ledState),
              buttonState(buttonState),
              boardConfig(boardConfig)
        {}

        esp_err_t init();

    protected:
        LEDState& ledState;
        ButtonState& buttonState;
        const AbstractBoardConfig& boardConfig;

        void tickMainboardSimulationMode();

        struct HandshakeMessageResponsePair {
            uint8_t expectedMessage[MAX_MESSAGE_LENGTH];
            uint8_t response[MAX_MESSAGE_LENGTH];
            size_t expectedSize;
            size_t responseSize;
            mutable bool sent;
        };

        bool handshakeSuccessful = false;
        uint8_t currentLEDMessage[MAX_MESSAGE_LENGTH] = {0};
        bool isSecondMessage = false;

        void tryHandshake();
        void processHandshake();
        void getHandshakeResponse(const uint8_t* message, size_t length, uint8_t* responseBuffer, size_t& responseLength);

        bool processCoverUIMessages();
        void sendMessagesAsMainboardWouldDo();

        bool readNextSerialMessage(uint8_t* messageBuffer, size_t& messageLength);

        void updateLEDStateMessage(bool forceUpdate = false);

        static const HandshakeMessageResponsePair handshakeMessageResponses[];
        static const int handshakeMessageResponsesCount;

        void updateButtonState(const uint8_t* messageBuffer, size_t messageLength);
        void onUARTReceive(const uint8_t* data, size_t length) override;
        void receiveMessageFromCoverUI(const uint8_t* data, size_t length) override;
        void responseToHandshakeMessage(const uint8_t* message, size_t length);
    private:
        // received messages buffer
        static constexpr size_t BUFFER_SIZE = 128;
        uint8_t messageBuffer[BUFFER_SIZE];
        size_t bufferLength = 0;

        void addMessageToBuffer(const uint8_t* message, size_t length);
        void processReceivedMessages();
        void processSingleMessage(const uint8_t* message, size_t length);
        void debugMessage(const uint8_t* message, size_t length);
    };

} // namespace YFComms