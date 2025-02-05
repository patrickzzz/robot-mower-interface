#include "VirtualMainboardUART.hpp"
#include "esp_log.h"
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace YFComms {
    const VirtualMainboardUART::HandshakeMessageResponsePair VirtualMainboardUART::handshakeMessageResponses[] = {
        {{0x55, 0xAA, 0x03, 0x40, 0x01, 0x00, 0x43}, {0x55, 0xAA, 0x02, 0xFF, 0xFF, 0xFF}, 7, 6, false},
        {{0x55, 0xAA, 0x02, 0xFF, 0xFF, 0xFF}, {0x55, 0xAA, 0x02, 0xFF, 0xFE, 0xFE}, 6, 6, false},
        {{0x55, 0xAA, 0x02, 0xFF, 0xFE, 0xFE}, {0x55, 0xAA, 0x05, 0xFF, 0xFD, 0x06, 0x50, 0x20, 0x76}, 6, 9, false},
        {{0x55, 0xAA, 0x03, 0xFF, 0xFD, 0x06, 0x04}, {0x55, 0xAA, 0x02, 0xFF, 0xFB, 0xFB}, 7, 6, false},
        {{0x55, 0xAA, 0x1B, 0xFF, 0xFB, 0x52, 0x4D, 0x20, 0x45, 0x43, 0x34, 0x5F, 0x56, 0x31, 0x2E, 0x30, 0x30, 0x5F, 0x32, 0x30, 0x32, 0x30, 0x28, 0x32, 0x30, 0x30, 0x39, 0x33, 0x30, 0x29, 0xA5}, {0x55, 0xAA, 0x02, 0xFF, 0xFB, 0xFB}, 31, 6, false},
        {{0x55, 0xAA, 0x1B, 0xFF, 0xFB, 0x52, 0x4D, 0x20, 0x45, 0x43, 0x34, 0x5F, 0x56, 0x31, 0x2E, 0x30, 0x30, 0x5F, 0x32, 0x30, 0x32, 0x30, 0x28, 0x32, 0x30, 0x30, 0x39, 0x33, 0x30, 0x29, 0xA5}, {0x55, 0xAA, 0x02, 0x00, 0x00, 0x01}, 31, 6, false},
        {{0x55, 0xAA, 0x03, 0x40, 0x01, 0x00, 0x43}, {0x0}, 7, 1, false}
    };

    constexpr int VirtualMainboardUART::handshakeMessageResponsesCount = sizeof(handshakeMessageResponses) / sizeof(handshakeMessageResponses[0]);

    esp_err_t VirtualMainboardUART::init() {
        ESP_LOGI(TAG, "Starting VirtualMainboardUART");

        for (int i = 0; i < handshakeMessageResponsesCount; i++) {
            handshakeMessageResponses[i].sent = false;
        }

        updateLEDStateMessage(true);
        AbstractUARTYF::sendStartSequence();

        return ESP_OK;
    }

    void VirtualMainboardUART::onUARTReceive(const uint8_t* data, size_t length) {
        // no implementation, as this virtual mainboard does not receive any messages from a hardware mainboard
    }

    void VirtualMainboardUART::tickMainboardSimulationMode() {
        ESP_LOGI(TAG, "VirtualMainboardUART task started");

        while (true) {
            if (!handshakeSuccessful) {
                tryHandshake();
            }

            sendMessagesAsMainboardWouldDo();

            while(processCoverUIMessages()) {
                // Process all incoming messages
            }

            vTaskDelay(75 / portTICK_PERIOD_MS);
        }
    }

    void VirtualMainboardUART::sendMessagesAsMainboardWouldDo() {
        updateLEDStateMessage();

        // LED Status Messages
        coverUIControllerUART->sendYFMessage(currentLEDMessage);

        // Request Button states message
        const uint8_t buttonStateRequest[] = {0x55, 0xAA, 0x02, 0x50, 0x62, 0xB3};
        coverUIControllerUART->sendYFMessage(buttonStateRequest);

        if(isSecondMessage) {
            // Some status, containing lock state etc.
            const uint8_t statusMessage[] = {0x55, 0xAA, 0x05, 0x50, 0x84, 0x00, 0xFF, 0x01, 0xD8};
            coverUIControllerUART->sendYFMessage(statusMessage);
        }
        isSecondMessage = !isSecondMessage;
    }

    bool VirtualMainboardUART::processCoverUIMessages() {
        uint8_t messageBuffer[AbstractUARTConnector::MAX_MESSAGE_LENGTH] = {0};
        size_t messageLength = 0;

        if (readNextSerialMessage(messageBuffer, messageLength)) {
            // 55 AA 03 40 01 00 43 <= Handshake message from CoverUI is coming up again in times, but can be ignored
            // 55 AA 03 50 84 0D E3 <= Some status message (50 84, also comes from maiboard)
            // only show 50 00 (button release) and 50 62 button press states
            if (messageBuffer[3] == 0x50 && messageBuffer[4] == 0x00) {
                return true;    // button release confirmation
            } else if (messageBuffer[3] == 0x50 && messageBuffer[4] == 0x84) {
                return true;    // ignore for now some status message
            } else if (messageBuffer[3] == 0x40 && messageBuffer[4] == 0x01) {
                return true;    // ignore also hand shake frame ping, that comes up now and then
            } else if(messageBuffer[3] == 0x50 && messageBuffer[4] == 0x62) {
                updateButtonState(messageBuffer, messageLength);
                return true;
            }

            // Convert message to hex string for logging
            char hexMessage[AbstractUARTConnector::MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
            size_t hexIndex = 0;
            for (int i = 0; i < messageLength && hexIndex < sizeof(hexMessage) - 3; i++) {
                snprintf(&hexMessage[hexIndex], 4, "%02X ", messageBuffer[i]);
                hexIndex += 3;
            }

            ESP_LOGI("UART Received", "Unknown: %s", hexMessage);
            /*
                log of more messages found:
                55 AA 02 FF FA FA => after some kind of error, leds went off.. but on button click, at least this came.

             */

            return true;
        }

        return false;
    }

    bool VirtualMainboardUART::readNextSerialMessage(uint8_t* messageBuffer, size_t& messageLength) {
        uint8_t message[AbstractUARTConnector::MAX_MESSAGE_LENGTH] = {0};
        size_t currentLength = 0;

        uint8_t data[1];
        int length;

        while ((length = uart_read_bytes(UART_NUM_1, data, 1, 10 / portTICK_PERIOD_MS)) > 0) {
            if (currentLength == 0 && data[0] != 0x55) {
                continue; // Message does not start with 0x55
            }

            if (currentLength < AbstractUARTConnector::MAX_MESSAGE_LENGTH) {
                message[currentLength++] = data[0];

                if (AbstractUARTYF::isCompleteMessage(message, currentLength)) {
                    if (!AbstractUARTYF::hasCorrectChecksum(message, currentLength)) {
                        ESP_LOGW(TAG, "Checksum is invalid, discarding message");
                        currentLength = 0;
                        memset(message, 0, AbstractUARTConnector::MAX_MESSAGE_LENGTH);
                        continue;
                    }

                    memcpy(messageBuffer, message, currentLength);
                    messageLength = currentLength;

                    return true;
                }
            } else {
                ESP_LOGW(TAG, "Message buffer overflow!");
                return false;
            }
        }

        return false;
    }



    void VirtualMainboardUART::tryHandshake() {
        while (!handshakeSuccessful) {
            ESP_LOGI(TAG, "Trying to do handshake to CoverUI");
            processHandshake();

            if(!handshakeSuccessful) {
                ESP_LOGE(TAG, "Handshake was not successfull! Retrying in 3 seconds..");
                vTaskDelay(pdMS_TO_TICKS(3000));
            }else{
                ESP_LOGI(TAG, "Handshake was successfull!");
            }
        }
    }

    void VirtualMainboardUART::processHandshake() {
        uint8_t messageBuffer[AbstractUARTConnector::MAX_MESSAGE_LENGTH] = {0};
        size_t messageLength = 0;

        // set all response sent flags to false
        for (int i = 0; i < handshakeMessageResponsesCount; i++) {
            handshakeMessageResponses[i].sent = false;
        }

        // handshake is initialized and closed by CoverUI
        // so we react to responses of the CoverUI
        while (!handshakeSuccessful) {
            if (readNextSerialMessage(messageBuffer, messageLength)) {
                uint8_t response[AbstractUARTConnector::MAX_MESSAGE_LENGTH] = {0};
                size_t responseLength = 0;
                getHandshakeResponse(messageBuffer, messageLength, response, responseLength);

                if(responseLength == 0) {
                    if(
                        messageBuffer[0] == 0x55 && messageBuffer[1] == 0xaa &&
                        messageBuffer[3] == 0x50 && messageBuffer[4] == 0x62) {
                            ESP_LOGI(TAG, "Received button state message, means, handshake had already been done before. Lets try to boot it up!");
                            handshakeSuccessful = true;
                            return;
                    }

                    ESP_LOGW(TAG, "Received handshake message was unknown.");
                    // hexdump
                    char hexMessage[AbstractUARTConnector::MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
                    size_t hexIndex = 0;
                    for (int i = 0; i < messageLength && hexIndex < sizeof(hexMessage) - 3; i++) {
                        snprintf(&hexMessage[hexIndex], 4, "%02X ", messageBuffer[i]);
                        hexIndex += 3;
                    }
                    ESP_LOGW(TAG, "Unknown: %s", hexMessage);

                    return;  // treat handshake as failed
                }else if(responseLength == 1 && response[0] == 0x00) {
                    // this is the end message after successful handshake
                    handshakeSuccessful = true;
                    return;
                }

                sendYFMessage(response);
            }
        }

        // handshake was not successfull.. Normally, CoverUI resends the handshake messages..
        vTaskDelay(pdMS_TO_TICKS(3000));
        return;
    }

    void VirtualMainboardUART::getHandshakeResponse(const uint8_t* message, size_t length, uint8_t* responseBuffer, size_t& responseLength) {
        for (int i = 0; i < handshakeMessageResponsesCount; i++) {
            if (memcmp(message, handshakeMessageResponses[i].expectedMessage, handshakeMessageResponses[i].expectedSize) == 0) {
                if (!handshakeMessageResponses[i].sent) {
                    handshakeMessageResponses[i].sent = true;
                } else {
                    continue;
                }

                memcpy(responseBuffer, handshakeMessageResponses[i].response, handshakeMessageResponses[i].responseSize);
                responseLength = handshakeMessageResponses[i].responseSize;

                return;
            }
        }

        responseLength = 0;
    }


    void VirtualMainboardUART::updateLEDStateMessage(bool forceUpdate) {
        if (!forceUpdate && !ledState.getIsUpdated()) {
            return;
        }

        size_t messageLength = 0;
        const uint8_t* defaultMessage = boardConfig.getDefaultLEDMessage(messageLength);

        if (defaultMessage == nullptr || messageLength == 0) {
            ESP_LOGW(TAG, "No LED message available for this board.");
            return;
        }

        uint8_t newLEDMessage[AbstractUARTConnector::MAX_MESSAGE_LENGTH] = {0};
        memcpy(newLEDMessage, defaultMessage, messageLength);

        size_t ledCount;
        const auto* leds = boardConfig.getLEDConfigs(ledCount);

        for (size_t i = 0; i < ledCount; i++) {
            if (leds[i].commType == AbstractBoardConfig::CommunicationType::UART) {
                uint8_t ledStateValue = static_cast<uint8_t>(ledState.getState(static_cast<LED>(leds[i].ledIndex)));
                newLEDMessage[5 + leds[i].uartMessagePos] = ledStateValue;
            }
        }

        AbstractUARTYF::updateChecksumInMessage(newLEDMessage, messageLength);
        memcpy(currentLEDMessage, newLEDMessage, messageLength);

        ledState.setIsUpdated(false);
    }

    void VirtualMainboardUART::updateButtonState(const uint8_t* messageBuffer, size_t messageLength) {
        size_t buttonCount;
        const auto* buttons = boardConfig.getButtonConfigs(buttonCount);

        for (size_t i = 0; i < buttonCount; i++) {
            if (buttons[i].commType == AbstractBoardConfig::CommunicationType::UART) {
                if(buttons[i].uartMessagePos > messageLength - 7) {
                    ESP_LOGW(TAG, "Message length is too short for button state update: %d", messageLength);
                    return;
                }
                uint8_t buttonStateValue = messageBuffer[5 + buttons[i].uartMessagePos];
                if(buttonStateValue == 0x00) {
                    buttonState.setState(static_cast<Button>(buttons[i].buttonIndex), ButtonStateEnum::LOW, 20);
                }else if(buttonStateValue == 0x02) {
                    buttonState.setState(static_cast<Button>(buttons[i].buttonIndex), ButtonStateEnum::HIGH, 20);
                }
            }
        }
    }

    void VirtualMainboardUART::responseToHandshakeMessage(const uint8_t* message, size_t length) {
        uint8_t response[AbstractUARTConnector::MAX_MESSAGE_LENGTH] = {0};
        size_t responseLength = 0;
        getHandshakeResponse(message, length, response, responseLength);

        if(responseLength == 0) {
            if(
                message[0] == 0x55 && message[1] == 0xaa &&
                message[3] == 0x50 && message[4] == 0x62) {
                    ESP_LOGI(TAG, "Received button state message, means, handshake had already been done before. Lets try to boot it up!");
                    handshakeSuccessful = true;
                    return;
            }

            ESP_LOGW(TAG, "Received handshake message was unknown.");
            // hexdump
            char hexMessage[AbstractUARTConnector::MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
            size_t hexIndex = 0;
            for (int i = 0; i < length && hexIndex < sizeof(hexMessage) - 3; i++) {
                snprintf(&hexMessage[hexIndex], 4, "%02X ", message[i]);
                hexIndex += 3;
            }
            ESP_LOGW(TAG, "Unknown: %s", hexMessage);

            return;  // treat handshake as failed
        }else if(responseLength == 1 && response[0] == 0x00) {
            ESP_LOGI(TAG, "Handshake was successfull!");
            // this is the end message after successful handshake
            handshakeSuccessful = true;

            sendMessagesAsMainboardWouldDo();

            return;
        }

        coverUIControllerUART->sendYFMessage(response);
    }

    void VirtualMainboardUART::receiveMessageFromCoverUI(const uint8_t* data, size_t length) {
        /*
        char hexMessage[AbstractUARTConnector::MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
        size_t hexIndex = 0;
        for (int i = 0; i < length && hexIndex < sizeof(hexMessage) - 3; i++) {
            snprintf(&hexMessage[hexIndex], 4, "%02X ", data[i]);
            hexIndex += 3;
        }

        ESP_LOGI("UART Received", "Mainboard received from CoverUI: %s", hexMessage);
        */

        if(!coverUIControllerUART) {
            ESP_LOGW(TAG, "No coverUIControllerUART set for VirtualMainboardUART");
            return;
        }

        if (!handshakeSuccessful) {
            responseToHandshakeMessage(data, length);
        }else{
            addMessageToBuffer(data, length);
            processReceivedMessages();

            vTaskDelay(75 / portTICK_PERIOD_MS);
            sendMessagesAsMainboardWouldDo();
        }
    }

    void VirtualMainboardUART::addMessageToBuffer(const uint8_t* message, size_t length) {
        if (bufferLength + length > BUFFER_SIZE) {
            ESP_LOGW(TAG, "Buffer overflow, resetting received msg buffer!");
            bufferLength = 0;
        }

        memcpy(&messageBuffer[bufferLength], message, length);
        bufferLength += length;
    }

void VirtualMainboardUART::processReceivedMessages() {
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

    void VirtualMainboardUART::processSingleMessage(const uint8_t* data, size_t length) {
        // 55 AA 03 40 01 00 43 <= Handshake message from CoverUI is coming up again in times, but can be ignored
        // 55 AA 03 50 84 0D E3 <= Some status message (50 84, also comes from maiboard)
        // only show 50 00 (button release) and 50 62 button press states
        if (data[3] == 0x50 && data[4] == 0x00) {
            debugMessage(data, length);
            return;    // button release confirmation
        } else if (data[3] == 0x50 && data[4] == 0x84) {
            debugMessage(data, length);
            return;    // ignore for now some status message
        } else if (data[3] == 0x40 && data[4] == 0x01) {
            debugMessage(data, length);
            return;    // ignore also hand shake frame ping, that comes up now and then
        } else if(data[3] == 0x50 && data[4] == 0x62) {
            updateButtonState(data, length);
            return;
        }

        debugMessage(data, length);
    }

    void VirtualMainboardUART::debugMessage(const uint8_t* message, size_t length) {
        char hexMessage[AbstractUARTConnector::MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
        size_t hexIndex = 0;
        for (int i = 0; i < length && hexIndex < sizeof(hexMessage) - 3; i++) {
            snprintf(&hexMessage[hexIndex], 4, "%02X ", message[i]);
            hexIndex += 3;
        }

        ESP_LOGI("UART Received", "Message: %s", hexMessage);
    }
} // namespace YFComms