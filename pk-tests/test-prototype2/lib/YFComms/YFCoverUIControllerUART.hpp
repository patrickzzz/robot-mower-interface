#ifndef YF_COMMS_COVER_UI_SERIAL_HPP
#define YF_COMMS_COVER_UI_SERIAL_HPP

#include <vector>
#include <string>
#include "driver/uart.h"
#include "LEDState.hpp"
#include "BoardConfig.hpp"
#include "LEDState.hpp"

namespace YFComms {
    class YFCoverUIControllerUART {

    public:
        YFCoverUIControllerUART(LEDState& ledState, const BoardConfig& boardConfig);

        static constexpr int MAX_MESSAGE_LENGTH = 256;
        static constexpr int TIMEOUT_DURATION = 5000;

        static constexpr int UART_RX_PIN = 38;
        static constexpr int UART_TX_PIN = 37;
        static constexpr int UART_BAUD_RATE = 115200;

        void start();
        void stop();

    private:
        LEDState& ledState;
        const BoardConfig& boardConfig;

        // Initialize UART
        void initializeUart();

        // Task-Handle for serial communication task
        TaskHandle_t serialTaskHandle;
        static void serialTask(void* pvParameters);

        void tick();

        struct HandshakeMessageResponsePair {
            std::vector<uint8_t> expectedMessage;
            std::vector<uint8_t> response;
            mutable bool sent;
        };

        bool stopped = false;
        bool handshakeSuccessful = false;
        std::vector<uint8_t> currentLEDMessage = {};

        void sendStartSequence();
        void tryHandshake();
        void processHandshake();
        void processCoverUIMessages();
        void processMainboardMessages();

        void updateLEDStateMessage(bool forceUpdate = false);

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