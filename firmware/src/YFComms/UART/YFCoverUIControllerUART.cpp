#include "YFCoverUIControllerUART.hpp"
#include "esp_log.h"
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace YFComms {
    constexpr char TAG[] = "YFCoverUIControllerUART";

    const YFCoverUIControllerUART::HandshakeMessageResponsePair YFCoverUIControllerUART::handshakeMessageResponses[] = {
        {{0x55, 0xAA, 0x03, 0x40, 0x01, 0x00, 0x43}, {0x55, 0xAA, 0x02, 0xFF, 0xFF, 0xFF}, 7, 6, false},
        {{0x55, 0xAA, 0x02, 0xFF, 0xFF, 0xFF}, {0x55, 0xAA, 0x02, 0xFF, 0xFE, 0xFE}, 6, 6, false},
        {{0x55, 0xAA, 0x02, 0xFF, 0xFE, 0xFE}, {0x55, 0xAA, 0x05, 0xFF, 0xFD, 0x06, 0x50, 0x20, 0x76}, 6, 9, false},
        {{0x55, 0xAA, 0x03, 0xFF, 0xFD, 0x06, 0x04}, {0x55, 0xAA, 0x02, 0xFF, 0xFB, 0xFB}, 7, 6, false},
        {{0x55, 0xAA, 0x1B, 0xFF, 0xFB, 0x52, 0x4D, 0x20, 0x45, 0x43, 0x34, 0x5F, 0x56, 0x31, 0x2E, 0x30, 0x30, 0x5F, 0x32, 0x30, 0x32, 0x30, 0x28, 0x32, 0x30, 0x30, 0x39, 0x33, 0x30, 0x29, 0xA5}, {0x55, 0xAA, 0x02, 0xFF, 0xFB, 0xFB}, 31, 6, false},
        {{0x55, 0xAA, 0x1B, 0xFF, 0xFB, 0x52, 0x4D, 0x20, 0x45, 0x43, 0x34, 0x5F, 0x56, 0x31, 0x2E, 0x30, 0x30, 0x5F, 0x32, 0x30, 0x32, 0x30, 0x28, 0x32, 0x30, 0x30, 0x39, 0x33, 0x30, 0x29, 0xA5}, {0x55, 0xAA, 0x02, 0x00, 0x00, 0x01}, 31, 6, false},
        {{0x55, 0xAA, 0x03, 0x40, 0x01, 0x00, 0x43}, {0x0}, 7, 1, false}
    };

    constexpr int YFCoverUIControllerUART::handshakeMessageResponsesCount = sizeof(handshakeMessageResponses) / sizeof(handshakeMessageResponses[0]);

    YFCoverUIControllerUART::YFCoverUIControllerUART(LEDState& ledState, ButtonState& buttonState, const AbstractBoardConfig& boardConfig)
     : ledState(ledState), buttonState(buttonState), boardConfig(boardConfig) {}

    void YFCoverUIControllerUART::start() {
        ESP_LOGI(TAG, "Starting YFCoverUIControllerUART");
        initializeUart();

        if (serialTaskHandle == nullptr) {
            ESP_LOGI(TAG, "Starting serial task");
            xTaskCreate(serialTask, "SerialTask", 4096, this, 10, &serialTaskHandle);
        }

        updateLEDStateMessage(true);
    }

    // ToDo: Check to do this in destructor
    void YFCoverUIControllerUART::stop() {
        if (serialTaskHandle != nullptr) {
            vTaskDelete(serialTaskHandle);
            serialTaskHandle = nullptr;
        }

        uart_driver_delete(UART_NUM_1);
    }

    void YFCoverUIControllerUART::initializeUart() {
        ESP_LOGI(TAG, "Initializing UART");

        uart_config_t uart_config = {
            .baud_rate = UART_BAUD_RATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_APB
        };
        uart_param_config(UART_NUM_1, &uart_config);
        uart_set_pin(UART_NUM_1, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        uart_driver_install(UART_NUM_1, 1024, 1024, 10, NULL, 0);
    }

    void YFCoverUIControllerUART::serialTask(void* pvParameters) {
        ESP_LOGI(TAG, "Starting YFCoverUIControllerUART serialTask");
        auto* instance = static_cast<YFCoverUIControllerUART*>(pvParameters);
        instance->tickMainboardSimulationMode();
    }

    void YFCoverUIControllerUART::tickMainboardSimulationMode() {
        sendStartSequence();
        ESP_LOGI(TAG, "YFCoverUIControllerUART task started");

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

    void YFCoverUIControllerUART::sendStartSequence() {
        // Send start sequence
        uint8_t tx_data[] = {0x00, 0xFF};
        uart_write_bytes(UART_NUM_1, (const char*)tx_data, sizeof(tx_data));
        ESP_LOGI("MAIN", "Start sequence sent: 0x00 0xFF");
    }

    void YFCoverUIControllerUART::sendMessagesAsMainboardWouldDo() {
        updateLEDStateMessage();

        // LED Status Messages
        sendMessage(currentLEDMessage);

        // Request Button states message
        const uint8_t buttonStateRequest[] = {0x55, 0xAA, 0x02, 0x50, 0x62, 0xB3};
        sendMessage(buttonStateRequest);

        if(isSecondMessage) {
            // Some status, containing lock state etc.
            const uint8_t statusMessage[] = {0x55, 0xAA, 0x05, 0x50, 0x84, 0x00, 0xFF, 0x01, 0xD8};
            sendMessage(statusMessage);
        }
        isSecondMessage = !isSecondMessage;
    }

    bool YFCoverUIControllerUART::processCoverUIMessages() {
        uint8_t messageBuffer[MAX_MESSAGE_LENGTH] = {0};
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
            char hexMessage[MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
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

    bool YFCoverUIControllerUART::readNextSerialMessage(uint8_t* messageBuffer, size_t& messageLength) {
        uint8_t message[MAX_MESSAGE_LENGTH] = {0};
        size_t currentLength = 0;

        uint8_t data[1];
        int length;

        while ((length = uart_read_bytes(UART_NUM_1, data, 1, 10 / portTICK_PERIOD_MS)) > 0) {
            if (currentLength == 0 && data[0] != 0x55) {
                continue; // Message does not start with 0x55
            }

            if (currentLength < MAX_MESSAGE_LENGTH) {
                message[currentLength++] = data[0];

                if (isCompleteMessage(message, currentLength)) {
                    if (!hasCorrectChecksum(message, currentLength)) {
                        ESP_LOGW(TAG, "Checksum is invalid, discarding message");
                        currentLength = 0;
                        memset(message, 0, MAX_MESSAGE_LENGTH);
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

    bool YFCoverUIControllerUART::isCompleteMessage(const uint8_t* message, size_t length) {
        if (length < 4) return false; // Minimum length is 4

        // check header
        int expectedLength = message[2] + 4; // message[2] = length of message, + 4 [55 AA Length {msg} Checksum]

        return (length >= expectedLength);
    }

    /*
     * The checksum is calculated such that the sum of all bytes (including the checksum)
     * modulo 256 equals zero. This ensures the integrity of the transmitted message.
     */
    bool YFCoverUIControllerUART::hasCorrectChecksum(const uint8_t* message, size_t length) {
        if (length < 4) {
            return false;
        }

        return message[length - 1] == calculateChecksum(message, length - 1);
    }

    void YFCoverUIControllerUART::addChecksumToMessage(uint8_t* message, size_t& length) {
        if (length + 1 > MAX_MESSAGE_LENGTH) {
            return;
        }

        message[length] = calculateChecksum(message, length);
        length++;
    }

    void YFCoverUIControllerUART::updateChecksumInMessage(uint8_t* message, size_t length) {
        if (length < 2) {
            return;
        }

        message[length - 1] = calculateChecksum(message, length - 1);
    }

    uint8_t YFCoverUIControllerUART::calculateChecksum(const uint8_t* message, size_t length) {
        uint16_t checksum = 0;

        for (size_t i = 0; i < length; i++) {
            checksum += message[i];
        }

        return static_cast<uint8_t>(checksum % 256);
    }

    void YFCoverUIControllerUART::tryHandshake() {
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

    void YFCoverUIControllerUART::processHandshake() {
        uint8_t messageBuffer[MAX_MESSAGE_LENGTH] = {0};
        size_t messageLength = 0;

        // set all response sent flags to false
        for (int i = 0; i < handshakeMessageResponsesCount; i++) {
            handshakeMessageResponses[i].sent = false;
        }

        // handshake is initialized and closed by CoverUI
        // so we react to responses of the CoverUI
        while (!handshakeSuccessful) {
            if (readNextSerialMessage(messageBuffer, messageLength)) {
                uint8_t response[MAX_MESSAGE_LENGTH] = {0};
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
                    char hexMessage[MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
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

                sendMessage(response);
            }
        }

        // handshake was not successfull.. Normally, CoverUI resends the handshake messages..
        vTaskDelay(pdMS_TO_TICKS(3000));
        return;
    }

    void YFCoverUIControllerUART::getHandshakeResponse(const uint8_t* message, size_t length, uint8_t* responseBuffer, size_t& responseLength) {
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

    void YFCoverUIControllerUART::sendMessage(const uint8_t* message) {
        if (message == nullptr || message[0] != 0x55 || message[1] != 0xAA) return;

        size_t length = message[2] + 4;

        /*
        // Format the message as hex string
        char hexMessage[length * 3 + 1]; // 2 Zeichen pro Byte + Leerzeichen + Nullterminator
        int pos = 0;
        for (int i = 0; i < length; i++) {
            pos += snprintf(&hexMessage[pos], sizeof(hexMessage) - pos, "%02X ", message[i]);
        }
        ESP_LOGI("UART SEND", "%s", hexMessage);
        */
        uart_write_bytes(UART_NUM_1, (const char*)message, length);
    }

    void YFCoverUIControllerUART::updateLEDStateMessage(bool forceUpdate) {
        if (!forceUpdate && !ledState.getIsUpdated()) {
            return;
        }

        size_t messageLength = 0;
        const uint8_t* defaultMessage = boardConfig.getDefaultLEDMessage(messageLength);

        if (defaultMessage == nullptr || messageLength == 0) {
            ESP_LOGW(TAG, "No LED message available for this board.");
            return;
        }

        uint8_t newLEDMessage[MAX_MESSAGE_LENGTH] = {0};
        memcpy(newLEDMessage, defaultMessage, messageLength);

        size_t ledCount;
        const auto* leds = boardConfig.getLEDConfigs(ledCount);

        for (size_t i = 0; i < ledCount; i++) {
            if (leds[i].commType == AbstractBoardConfig::CommunicationType::UART) {
                uint8_t ledStateValue = static_cast<uint8_t>(ledState.getState(static_cast<LED>(leds[i].ledIndex)));
                newLEDMessage[5 + leds[i].uartMessagePos] = ledStateValue;
            }
        }

        updateChecksumInMessage(newLEDMessage, messageLength);
        memcpy(currentLEDMessage, newLEDMessage, messageLength);

        ledState.setIsUpdated(false);
    }

    void YFCoverUIControllerUART::updateButtonState(uint8_t* messageBuffer, size_t messageLength) {
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
} // namespace YFComms