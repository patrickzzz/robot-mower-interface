#ifndef YF_COMMS_COVER_UI_SERIAL_HPP
#define YF_COMMS_COVER_UI_SERIAL_HPP

#include <vector>
#include <string>
#include "driver/uart.h"

namespace YFComms {

    class YFCommsCoverUISerial {
    public:
        static constexpr int MAX_MESSAGE_LENGTH = 256;
        static constexpr int TIMEOUT_DURATION = 5000;

        static constexpr int UART_RX_PIN = 38;
        static constexpr int UART_TX_PIN = 37;
        static constexpr int UART_BAUD_RATE = 115200;

        YFCommsCoverUISerial();
        static void task(void* pvParameters);
        void initializeUart();
        void appMain();

    private:
        struct HandshakeMessageResponsePair {
            std::vector<uint8_t> expectedMessage;
            std::vector<uint8_t> response;
            mutable bool sent;
        };

        bool handshakeSuccessful = false;
        std::vector<uint8_t> defaultStatusMessage;
        std::vector<uint8_t> currentStatusMessage;

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
        void sendMessage(const std::vector<uint8_t>& response);

        static const HandshakeMessageResponsePair handshakeMessageResponses[];
        static const int handshakeMessageResponsesCount;
    };

} // namespace YFComms

#endif // YF_COMMS_COVER_UI_SERIAL_HPP