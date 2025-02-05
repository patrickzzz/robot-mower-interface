#include "CoverUIControllerUART.hpp"
#include "esp_log.h"
#include <cstring>

namespace YFComms {
    void CoverUIControllerUART::onUARTReceive(const uint8_t* data, size_t length) {

        if(mainboard) {
            mainboard->receiveMessageFromCoverUI(data, length);
            return;
        }

        // just debug message, as no mainboard is attached
        char hexMessage[AbstractUARTConnector::MAX_MESSAGE_LENGTH * 3] = {0}; // Each byte is max "FF " (3 chars)
        size_t hexIndex = 0;
        for (int i = 0; i < length && hexIndex < sizeof(hexMessage) - 3; i++) {
            snprintf(&hexMessage[hexIndex], 4, "%02X ", data[i]);
            hexIndex += 3;
        }

        ESP_LOGI("UART Received", "MSG from CoverUI, but no mainboard connected: %s", hexMessage);
    }

    void CoverUIControllerUART::receiveMessageFromMainboard(const uint8_t* data, size_t length) {
        sendMessage(data, length);
    }

} // namespace YFComms