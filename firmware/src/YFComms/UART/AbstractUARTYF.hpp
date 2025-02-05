#pragma once

#include "AbstractUARTConnector.hpp"
#include <vector>
#include <cstdint>
#include <esp_log.h>
#include "esp_check.h"


namespace YFComms {

    class AbstractUARTYF : public AbstractUARTConnector {
    public:
        AbstractUARTYF(uart_port_t uartPort, gpio_num_t rxPin, gpio_num_t txPin)
            : AbstractUARTConnector(uartPort, rxPin, txPin, 115200) {}

        esp_err_t sendYFMessage(const uint8_t* message) {
            if (message == nullptr || message[0] != 0x55 || message[1] != 0xAA) return ESP_ERR_INVALID_ARG;
            size_t length = message[2] + 4;    // message[2] = length of message, + 4 [55 AA Length {msg} Checksum]
/*
            // debug the message, that is being sent
            char hexMessage[AbstractUARTConnector::MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
            size_t hexIndex = 0;
            for (int i = 0; i < length && hexIndex < sizeof(hexMessage) - 3; i++) {
                snprintf(&hexMessage[hexIndex], 4, "%02X ", message[i]);
                hexIndex += 3;
            }

            ESP_LOGI("UART Sent", "Sending message: %s", hexMessage);
*/
            return AbstractUARTConnector::sendMessage(message, length);
        }

    protected:
        // Check if the message is complete
        bool isCompleteMessage(const uint8_t* message, size_t length) {
            if (length < 4) return false; // Minimum length is 4

            // check header
            int expectedLength = message[2] + 4; // message[2] = length of message, + 4 [55 AA Length {msg} Checksum]

            return (length >= expectedLength);
        }

        /*
         * The checksum is calculated such that the sum of all bytes (including the checksum)
         * modulo 256 equals zero. This ensures the integrity of the transmitted message.
         */
        bool hasCorrectChecksum(const uint8_t* message, size_t length) {
            if (length < 4) {
                return false;
            }

            return message[length - 1] == calculateChecksum(message, length - 1);
        }

        // Add checksum to message
        void addChecksumToMessage(uint8_t* message, size_t& length) {
            if (length + 1 > MAX_MESSAGE_LENGTH) {
                return;
            }

            message[length] = calculateChecksum(message, length);
            length++;
        }

        // Update checksum in message
        void updateChecksumInMessage(uint8_t* message, size_t length) {
            if (length < 2) {
                return;
            }

            message[length - 1] = calculateChecksum(message, length - 1);
        }

        // Calculate checksum
        uint8_t calculateChecksum(const uint8_t* message, size_t length) {
            uint16_t checksum = 0;

            for (size_t i = 0; i < length; i++) {
                checksum += message[i];
            }

            return static_cast<uint8_t>(checksum % 256);
        }

        // Start-Sequence and Handshake
        esp_err_t sendStartSequence() {
            uint8_t tx_data[] = {0x00, 0xFF};
            //uart_write_bytes(UART_NUM_1, (const char*)tx_data, sizeof(tx_data));
            return AbstractUARTConnector::sendMessage(tx_data, sizeof(tx_data));
        }

    };
} // namespace YFComms