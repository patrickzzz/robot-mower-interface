#include "HardwareMainboardUART.hpp"
#include "esp_log.h"
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace YFComms {

    void HardwareMainboardUART::onUARTReceive(const uint8_t* data, size_t length) {
        char hexMessage[AbstractUARTConnector::MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
        size_t hexIndex = 0;
        for (int i = 0; i < length && hexIndex < sizeof(hexMessage) - 3; i++) {
            snprintf(&hexMessage[hexIndex], 4, "%02X ", data[i]);
            hexIndex += 3;
        }

        ESP_LOGI("UART Received", "From Mainboard to CoverUI: %s", hexMessage);

        coverUIControllerUART->receiveMessageFromMainboard(data, length);
    }

    void HardwareMainboardUART::receiveMessageFromCoverUI(const uint8_t* data, size_t length) {
        /*
        char hexMessage[AbstractUARTConnector::MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
        size_t hexIndex = 0;
        for (int i = 0; i < length && hexIndex < sizeof(hexMessage) - 3; i++) {
            snprintf(&hexMessage[hexIndex], 4, "%02X ", data[i]);
            hexIndex += 3;
        }

        ESP_LOGI("UART Received", "Mainboard received from CoverUI: %s", hexMessage);
        */

        sendMessage(data, length);

        addMessageToBuffer(data, length);
        processReceivedMessages();
    }


    void HardwareMainboardUART::addMessageToBuffer(const uint8_t* message, size_t length) {
        if (bufferLength + length > BUFFER_SIZE) {
            ESP_LOGW(TAG, "Buffer overflow, resetting received msg buffer!");
            bufferLength = 0;
        }

        memcpy(&messageBuffer[bufferLength], message, length);
        bufferLength += length;
    }

    void HardwareMainboardUART::processReceivedMessages() {
        size_t i = 0;

        while (i + 2 <= bufferLength) {
            if (messageBuffer[i] == 0x55 && messageBuffer[i + 1] == 0xAA) {
                if (i + 3 > bufferLength) {
                    break;
                }

                uint8_t messageLength = messageBuffer[i + 2];

                if (i + 3 + messageLength > bufferLength) {
                    break;
                }

                processSingleMessage(&messageBuffer[i], messageLength + 4);

                i += messageLength + 4;
            } else {
                i++;
            }
        }

        if (i > 0 && i < bufferLength) {
            memmove(messageBuffer, &messageBuffer[i], bufferLength - i);
            bufferLength -= i;
        } else if (i >= bufferLength) {
            bufferLength = 0;
        }
    }

    void HardwareMainboardUART::processSingleMessage(const uint8_t* data, size_t length) {
        // 55 AA 03 40 01 00 43 <= Handshake message from CoverUI is coming up again in times, but can be ignored
        // 55 AA 03 50 84 0D E3 <= Some status message (50 84, also comes from maiboard)
        // only show 50 00 (button release) and 50 62 button press states
        if (data[3] == 0x50 && data[4] == 0x00) {
            //debugMessage(data, length);
            //return;    // button release confirmation
        } else if (data[3] == 0x50 && data[4] == 0x84) {
            //debugMessage(data, length);
            //return;    // ignore for now some status message
        } else if (data[3] == 0x40 && data[4] == 0x01) {
            //debugMessage(data, length);
            //return;    // ignore also hand shake frame ping, that comes up now and then
        } else if(data[3] == 0x50 && data[4] == 0x62) {
            //debugMessage(data, length);
            //updateButtonState(data, length);
            //return;
        }

        debugMessage(data, length);
    }

    void HardwareMainboardUART::debugMessage(const uint8_t* message, size_t length) {
        char hexMessage[AbstractUARTConnector::MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
        size_t hexIndex = 0;
        for (int i = 0; i < length && hexIndex < sizeof(hexMessage) - 3; i++) {
            snprintf(&hexMessage[hexIndex], 4, "%02X ", message[i]);
            hexIndex += 3;
        }

        ESP_LOGI("UART Received", "Message from CoverUI: %s", hexMessage);
    }
} // namespace YFComms