#include "YFCommsCoverUISerial.hpp"
#include "esp_log.h"
#include <cstring>
#include "CoverUILEDState.hpp"

namespace YFComms {

    constexpr char TAG[] = "YFCommsCoverUISerial";

    const YFCommsCoverUISerial::HandshakeMessageResponsePair YFCommsCoverUISerial::handshakeMessageResponses[] = {
        {{0x55, 0xAA, 0x03, 0x40, 0x01, 0x00, 0x43}, {0x55, 0xAA, 0x02, 0xFF, 0xFF, 0xFF}, false},
        {{0x55, 0xAA, 0x02, 0xFF, 0xFF, 0xFF}, {0x55, 0xAA, 0x02, 0xFF, 0xFE, 0xFE}, false},
        {{0x55, 0xAA, 0x02, 0xFF, 0xFE, 0xFE}, {0x55, 0xAA, 0x05, 0xFF, 0xFD, 0x06, 0x50, 0x20, 0x76}, false},
        {{0x55, 0xAA, 0x03, 0xFF, 0xFD, 0x06, 0x04}, {0x55, 0xAA, 0x02, 0xFF, 0xFB, 0xFB}, false},
        {{0x55, 0xAA, 0x1B, 0xFF, 0xFB, 0x52, 0x4D, 0x20, 0x45, 0x43, 0x34, 0x5F, 0x56, 0x31, 0x2E, 0x30, 0x30, 0x5F, 0x32, 0x30, 0x32, 0x30, 0x28, 0x32, 0x30, 0x30, 0x39, 0x33, 0x30, 0x29, 0xA5}, {0x55, 0xAA, 0x02, 0xFF, 0xFB, 0xFB}, false},
        {{0x55, 0xAA, 0x1B, 0xFF, 0xFB, 0x52, 0x4D, 0x20, 0x45, 0x43, 0x34, 0x5F, 0x56, 0x31, 0x2E, 0x30, 0x30, 0x5F, 0x32, 0x30, 0x32, 0x30, 0x28, 0x32, 0x30, 0x30, 0x39, 0x33, 0x30, 0x29, 0xA5}, {0x55, 0xAA, 0x02, 0x00, 0x00, 0x01}, false},
        {{0x55, 0xAA, 0x03, 0x40, 0x01, 0x00, 0x43}, {0x0}, false}
    };

    const int YFCommsCoverUISerial::handshakeMessageResponsesCount = sizeof(handshakeMessageResponses) / sizeof(handshakeMessageResponses[0]);

    YFCommsCoverUISerial::YFCommsCoverUISerial()
    : currentStatusMessage{}
    {}

    void YFCommsCoverUISerial::initializeUart() {
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
        uart_driver_install(UART_NUM_1, MAX_MESSAGE_LENGTH * 2, 0, 0, NULL, 0);
    }

    void YFCommsCoverUISerial::task(void* pvParameters) {
        // Cast pvParameters on class and call appMain()
        auto* instance = static_cast<YFCommsCoverUISerial*>(pvParameters);
        instance->appMain();
    }

    void YFCommsCoverUISerial::appMain() {
        initializeUart();

        ESP_LOGI(TAG, "Starting application...");

        CoverUILEDState uiLEDManager;
        /*
        uiLEDManager.setLEDState(0, CoverUILEDState::LEDState::ON);
        uiLEDManager.setLEDState(1, CoverUILEDState::LEDState::FLASH_SLOW);
        uiLEDManager.setLEDState(2, CoverUILEDState::LEDState::FLASH_FAST);
        uiLEDManager.setLEDState(3, CoverUILEDState::LEDState::FLASH_FAST);
        uiLEDManager.setLEDState(4, CoverUILEDState::LEDState::FLASH_SLOW);
         */
        uiLEDManager.setLEDState(17, CoverUILEDState::LEDState::FLASH_SLOW);
        uiLEDManager.setLEDState(18, CoverUILEDState::LEDState::ON);
        currentStatusMessage = uiLEDManager.getStatusMessage();
        addChecksumToMessage(currentStatusMessage);

        sendStartSequence();

        while (true) {
            if (!handshakeSuccessful) {
                tryHandshake();
            }

            processCoverUIMessages();
            processMainboardMessages();

            vTaskDelay(25 / portTICK_PERIOD_MS);
        }
    }

    void YFCommsCoverUISerial::sendStartSequence() {
        // Send start sequence
        uint8_t tx_data[] = {0x00, 0xFF};
        uart_write_bytes(UART_NUM_1, (const char*)tx_data, sizeof(tx_data));
        ESP_LOGI("MAIN", "Start sequence sent: 0x00 0xFF");
    }

    void YFCommsCoverUISerial::processMainboardMessages() {
        // LED Status Messages
        sendMessage(currentStatusMessage);

        // Request Button states message
        sendMessage({
            0x55, 0xAA, 0x02, 0x50, 0x62, 0xB3
        });

        // Some status, containing lock state etc.
        sendMessage({
            0x55, 0xAA, 0x05, 0x50, 0x84, 0x00, 0xFF, 0x01, 0xD8, 0x01
        });
    }

    void YFCommsCoverUISerial::processCoverUIMessages() {
        char messageBuffer[MAX_MESSAGE_LENGTH];
        memset(messageBuffer, 0, MAX_MESSAGE_LENGTH);
        int messageLength = 0;

        if (processIncomingSerialMessages(UART_NUM_1, messageBuffer, messageLength)) {
            std::string hexMessage;
            int i = 0;
            for (auto byte : messageBuffer) {
                char hex[4];  // Platz für "FF " + Nullterminator
                snprintf(hex, sizeof(hex), "%02X ", byte);
                hexMessage += hex;

                if(++i >= messageLength) {
                    break;
                }
            }

            // Hex-Daten als eine Zeile loggen
            ESP_LOGI("UART Received", "%s", hexMessage.c_str());

            // ...
        }
    }

    bool YFCommsCoverUISerial::processIncomingSerialMessages(uart_port_t uart_num, char *messageBuffer, int &messageLength) {
        static char message[MAX_MESSAGE_LENGTH];
        static int currentLength = 0;
        static TickType_t lastReceiveTime = 0;

        uint8_t data[1]; // Buffer für ein einzelnes Byte
        int length;

        while ((length = uart_read_bytes(uart_num, data, 1, 10 / portTICK_PERIOD_MS)) > 0) {
            // debug output msg
            lastReceiveTime = xTaskGetTickCount();

            if (currentLength == 0 && data[0] != 0x55) {
                continue; // Nachricht beginnt nicht mit 0x55, verwerfen
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

                    currentLength = 0;
                    memset(message, 0, MAX_MESSAGE_LENGTH);

                    return true;
                }
            } else {
                ESP_LOGW("UART", "Message buffer overflow! Message was: ");
                for (int i = 0; i < currentLength; i++) {
                    ESP_LOGW("UART", "0x%02X ", message[i]);
                }
                currentLength = 0;
                memset(message, 0, MAX_MESSAGE_LENGTH);
            }
        }

        // Timeout prüfen
        if ((xTaskGetTickCount() - lastReceiveTime) * portTICK_PERIOD_MS > TIMEOUT_DURATION && currentLength > 0) {
            ESP_LOGW("UART", "Message complete due to timeout: ");
            for (int i = 0; i < currentLength; i++) {
                ESP_LOGW("UART", "0x%02X ", message[i]);
            }
            currentLength = 0;
        }

        return false;
    }

    bool YFCommsCoverUISerial::isCompleteMessage(const char* message, int length) {
        if (length < 4) return false; // Minimum length is 4

        // check header
        int expectedLength = message[2] + 4; // message[2] = length of message, + 4 [55 AA Length {msg} Checksum]
        return (length >= expectedLength);

        return false;
    }

    /*
     * The checksum is calculated such that the sum of all bytes (including the checksum)
     * modulo 256 equals zero. This ensures the integrity of the transmitted message.
     */
    bool YFCommsCoverUISerial::hasCorrectChecksum(const char* message, int length) {
        if (length < 4) return false; // Minimum length for valid message is 4 bytes

        // Sum of all bytes except the last one (checksum)
        int calculatedChecksum = 0;
        for (int i = 0; i < length - 1; ++i) {
            calculatedChecksum += static_cast<unsigned char>(message[i]);
        }

        // Calculate checksum using modulo 256
        calculatedChecksum = calculatedChecksum % 256; // Instead of using & 0xFF, use modulo 256
        int messageChecksum = static_cast<unsigned char>(message[length - 1]);

        if(calculatedChecksum == messageChecksum) {
            return true;
        }else{
            return false;
        }
    }

    void YFCommsCoverUISerial::addChecksumToMessage(std::vector<uint8_t>& message) {
        int checksum = 0;

        for (size_t i = 0; i < message.size(); i++) {
            checksum += message[i];
        }

        checksum = checksum % 256;
        message.push_back(static_cast<uint8_t>(checksum));
    }

    void YFCommsCoverUISerial::tryHandshake() {
        while (!handshakeSuccessful) {
            ESP_LOGI(TAG, "Trying to do handshake to CoverUI");
            processHandshake();

            if(!handshakeSuccessful) {
                ESP_LOGE(TAG, "Handshake was not successfull! Retrying in 3 seconds..");
                vTaskDelay(3000 / portTICK_PERIOD_MS);

                // led_red_seq.blink({.on_ms = 50, .off_ms = 50, .limit_blink_cycles = 5, .fulfill = true});  // Default = 200ms ON, 200ms OFF
            }else{
                ESP_LOGI(TAG, "Handshake was successfull!");
                // led_grn_seq.blink({.on_ms = 50, .off_ms = 50, .limit_blink_cycles = 5, .fulfill = true});  // Default = 200ms ON, 200ms OFF
            }
        }
    }

    void YFCommsCoverUISerial::processHandshake() {
        char messageBuffer[MAX_MESSAGE_LENGTH];
        int messageLength = 0;

        // set all response sent flags to false
        for (int i = 0; i < handshakeMessageResponsesCount; i++) {
            handshakeMessageResponses[i].sent = false;
        }

        // handshake is initialized and closed by CoverUI
        // so we react to responses of the CoverUI
        while (!handshakeSuccessful) {
            if (processIncomingSerialMessages(UART_NUM_1, messageBuffer, messageLength)) {
                ESP_LOG_BUFFER_HEX("UART RECEIVED", messageBuffer, messageLength);
                std::vector<uint8_t> response = getHandshakeResponse(messageBuffer, messageLength);

                if(response.size() == 0) {
                    //Serial.print("Received handshake message was unknown");
                    ESP_LOGW(TAG, "Received handshake message was unknown");
                    /*
                    for (int i = 0; i < messageLength; i++) {
                        Serial.print(messageBuffer[i], HEX);
                        Serial.print(" ");
                    }
                    Serial.println();
                    */
                    return;  // treat handshake as failed
                }else if(response.size() == 1 && response[0] == 0x0) {
                    handshakeSuccessful = true;
                    return;
                }

                sendMessage(response);
            }
        }

        // handshake was not successfull.. Normally, CoverUI resends the handshake messages..
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        return;
    }

    std::vector<uint8_t> YFCommsCoverUISerial::getHandshakeResponse(const char* message, int length) {
        for (int i = 0; i < handshakeMessageResponsesCount; i++) {
            bool isMatch = memcmp(message, handshakeMessageResponses[i].expectedMessage.data(), handshakeMessageResponses[i].expectedMessage.size()) == 0;
            if (isMatch) {
                if(!handshakeMessageResponses[i].sent) {
                    handshakeMessageResponses[i].sent = true;
                } else {
                    continue;
                }

                return handshakeMessageResponses[i].response;
            }
        }

        return {};
    }


    void YFCommsCoverUISerial::sendMessage(const std::vector<uint8_t> &response) {
        // Nachricht in Hex formatieren
        std::string hexMessage;
        for (auto byte : response) {
            char hex[4];  // Platz für "FF " + Nullterminator
            snprintf(hex, sizeof(hex), "%02X ", byte);
            hexMessage += hex;
        }

        // Hex-Daten als eine Zeile loggen
        ESP_LOGI("UART SEND", "%s", hexMessage.c_str());

        // Nachricht senden
        uart_write_bytes(UART_NUM_1, (const char*)response.data(), response.size());
    }
} // namespace YFComms