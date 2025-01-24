#pragma once

#include <stdint.h>

#include "esp_log.h"
#include "mainboard_abstract_uart.hpp"

namespace mainboards {

constexpr int baud = 115200;

/**
 * @brief UART event task handler required by MainboardAbstractUART.
 *        Get called by UARTs event handler whenever data get received and need to be processed, as quick as possible.
 *
 * @param arg
 */
static void uart_event_task(void* arg) {
    constexpr size_t rx_pkt_buf_size = 200;  // FIXME: 100 byte should be enough. Test when having enough data or testsuite ready
    uart_event_t event;
    uint8_t* pkt_buf = (uint8_t*)malloc(rx_pkt_buf_size);

    for (;;) {
        // Waiting for UART event.
        if (!xQueueReceive(queue_, (void*)&event, (TickType_t)portMAX_DELAY))
            continue;

        bzero(pkt_buf, rx_pkt_buf_size);  // FIXME: Is this really necessary? In my point of view it will only waste resources!

        // Handle UART events
        switch (event.type) {
            case UART_DATA:
                ESP_LOGI(TAG, "[UART DATA]: %d bytes", event.size);
                uart_read_bytes(UART_NUM_2, pkt_buf, event.size, portMAX_DELAY);  // FIXME: UART port need to go to arg(s)
                ESP_LOG_BUFFER_HEXDUMP(TAG, pkt_buf, event.size, ESP_LOG_INFO);
                break;
            case UART_FIFO_OVF:
                ESP_LOGW(TAG, "UART_FIFO_OVF");
                uart_flush_input(UART_NUM_2);  // FIXME: UART port need to go to arg(s)
                xQueueReset(queue_);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGW(TAG, "Unhandled event UART_BUFFER_FULL");
                uart_flush_input(UART_NUM_2);  // FIXME: UART port need to go to arg(s)
                xQueueReset(queue_);
                break;
            case UART_BREAK:
                ESP_LOGW(TAG, "Unhandled event UART_BREAK");
                break;
            case UART_PARITY_ERR:
                ESP_LOGW(TAG, "Unhandled event UART_PARITY_ERR");
                break;
            case UART_FRAME_ERR:
                ESP_LOGW(TAG, "Unhandled event UART_FRAME_ERR");
                break;
            case UART_PATTERN_DET:
                ESP_LOGW(TAG, "Unhandled event UART_PATTERN_DET");
                break;
            case UART_EVENT_MAX:
                ESP_LOGW(TAG, "Unhandled event UART_EVENT_MAX");
                break;
            default:
                ESP_LOGW(TAG, "Unknown event type: %d", event.type);
                break;
        }
    }
    free(pkt_buf);
    pkt_buf = nullptr;
    vTaskDelete(NULL);
}

class MainboardDriverOM : public MainboardAbstractUART {
   public:
    MainboardDriverOM(const uart_port_t port, const gpio_num_t rx_pin, const gpio_num_t tx_pin, const void* on_event_cb)
        : MainboardAbstractUART(port, rx_pin, tx_pin, uart_event_task, on_event_cb, baud) {}
    ~MainboardDriverOM() {
        // TODO: Free COBS buffer
    }

    esp_err_t init() override {
        ESP_LOGI(TAG, "MainboardDriverOM::init()");
        esp_err_t ret = MainboardAbstractUART::init();
        // TODO: COBS buffer (PSRAM? instead if heap? but slower)
        // TODO: onCobsPacketReceive()
        return ret;
    }

    esp_err_t sendEmergency() override { return true; }
    esp_err_t sendButton(const uint8_t btn_id, const uint16_t duration) override { return true; }
    esp_err_t sendAlive() override { return true; }

   private:
    void onCobsPacketReceive() {
        // Decode COBS packet
    }
    void onPacketReceive() {
        // Decode and handle packet
        // Prepare packet for onMessageReceiveCB_
        // Call onMessageReceiveCB_
    }
    bool sendPacket(const void* packet, const size_t size) {
        // COBS encode packet
        // Send packet
        return true;
    }
};

}  // namespace mainboards
