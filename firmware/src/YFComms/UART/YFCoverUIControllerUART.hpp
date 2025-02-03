#pragma once

#include <string>
#include "driver/uart.h"
#include "../State/LEDState.hpp"
#include "../State/ButtonState.hpp"
#include "../BoardConfig/AbstractBoardConfig.hpp"

namespace YFComms {
    class YFCoverUIControllerUART {

    public:
        YFCoverUIControllerUART(LEDState& ledState, ButtonState& buttonState, const AbstractBoardConfig& boardConfig);

        static constexpr int MAX_MESSAGE_LENGTH = 64;

        static constexpr int UART_RX_PIN = 38;
        static constexpr int UART_TX_PIN = 37;
        static constexpr int UART_BAUD_RATE = 115200;

        void start();
        void stop();

    private:
        LEDState& ledState;
        ButtonState& buttonState;
        const AbstractBoardConfig& boardConfig;

        // Initialize UART
        void initializeUart();

        // Task-Handle for serial communication
        TaskHandle_t serialTaskHandle = nullptr;
        static void serialTask(void* pvParameters);

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

        void sendStartSequence();
        void tryHandshake();
        void processHandshake();
        bool processCoverUIMessages();
        void sendMessagesAsMainboardWouldDo();

        void getHandshakeResponse(const uint8_t* message, size_t length, uint8_t* responseBuffer, size_t& responseLength);
        bool readNextSerialMessage(uint8_t* messageBuffer, size_t& messageLength);
        bool isCompleteMessage(const uint8_t* message, size_t length);

        bool hasCorrectChecksum(const uint8_t* message, size_t length);
        void addChecksumToMessage(uint8_t* message, size_t& length);
        void updateChecksumInMessage(uint8_t* message, size_t length);
        uint8_t calculateChecksum(const uint8_t* message, size_t length);

        void sendMessage(const uint8_t* response);
        void updateLEDStateMessage(bool forceUpdate = false);

        static const HandshakeMessageResponsePair handshakeMessageResponses[];
        static const int handshakeMessageResponsesCount;

        void updateButtonState(uint8_t* messageBuffer, size_t messageLength);
    };

} // namespace YFComms