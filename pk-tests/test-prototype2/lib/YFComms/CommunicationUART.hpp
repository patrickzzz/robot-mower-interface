#ifndef YF_COMMS_COVER_UI_SERIAL_HPP
#define YF_COMMS_COVER_UI_SERIAL_HPP

#include <vector>
#include <string>
#include "driver/uart.h"
#include "LEDState.hpp"

namespace YFComms {

    class CommunicationUART {
    public:
        // Singleton: only way to get an instance of CommunicationUART
        static CommunicationUART& getInstance();

        static constexpr int MAX_MESSAGE_LENGTH = 256;
        static constexpr int TIMEOUT_DURATION = 5000;

        static constexpr int UART_RX_PIN = 38;
        static constexpr int UART_TX_PIN = 37;
        static constexpr int UART_BAUD_RATE = 115200;

        static void task(void* pvParameters);
        void initializeUart();
        void run();
        void setLEDStateInMessage(uint8_t ledIndex, LEDStateEnum state);

    private:
        // Private Construktor (Singleton)
        CommunicationUART() = default;
        CommunicationUART(const CommunicationUART&) = delete; // prohbit copy constructor
        CommunicationUART& operator=(const CommunicationUART&) = delete; // prohbit copy assignment

        struct HandshakeMessageResponsePair {
            std::vector<uint8_t> expectedMessage;
            std::vector<uint8_t> response;
            mutable bool sent;
        };

        bool isInitialized = false;
        bool handshakeSuccessful = false;
        std::vector<uint8_t> defaultLEDMessage = {0x55, 0xAA, 0x15, 0x50, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        std::vector<uint8_t> currentLEDMessage = {};

        void sendStartSequence();
        void tryHandshake();
        void processHandshake();
        void processCoverUIMessages();
        void processMainboardMessages();

        std::vector<uint8_t> getHandshakeResponse(const char* message, int length);
        bool processIncomingSerialMessages(uart_port_t uart_num, char* messageBuffer, int& messageLength);
        bool isCompleteMessage(const char* message, int length);
        bool hasCorrectChecksum(const char* message, int length);
        void addChecksumToMessage(std::vector<uint8_t>& message);
        void updateChecksumInMessage(std::vector<uint8_t>& message);
        void sendMessage(const std::vector<uint8_t>& response);


        static const HandshakeMessageResponsePair handshakeMessageResponses[];
        static const int handshakeMessageResponsesCount;
    };

} // namespace YFComms

#endif // YF_COMMS_COVER_UI_SERIAL_HPP