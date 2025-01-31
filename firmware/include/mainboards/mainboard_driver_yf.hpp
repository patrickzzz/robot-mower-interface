#pragma once

#include <stdint.h>

#include "esp_check.h"
#include "esp_log.h"
#include "mainboard_abstract_uart.hpp"

extern LedSequencer led_red_seq;

namespace mainboard_yf {
constexpr char TAG[] = "mainboard_yf";

constexpr int baud = 115200;
constexpr size_t rx_tx_buf_size = 100;               // UART receive buffer size

class MainboardDriverYF : public mainboards::MainboardAbstractUART {
   public:
    MainboardDriverYF(const uart_port_t port, const gpio_num_t rx_pin, const gpio_num_t tx_pin, const void* on_event_cb)
        : MainboardAbstractUART(port, rx_pin, tx_pin, on_event_cb, baud) {}
    ~MainboardDriverYF() {
        // Cleanup FreeRTOS queue
        if (event_task_handle_ != nullptr) {
            vTaskDelete(event_task_handle_);
        }
    }

    esp_err_t init() override {
        ESP_LOGI(TAG, "MainboardDriverYF::init()");

        if (esp_err_t ret = MainboardAbstractUART::init() != ESP_OK) {
            led_red_seq.blink({.limit_blink_cycles = 4, .fulfill = true});
            return ret;
        }

        // Create a task to handler UART event from ISR
        xTaskCreate(uart_event_task, "uart_event_task", 4096, (void*)this, 12, &event_task_handle_);
        if (event_task_handle_ == nullptr) {
            ESP_LOGE(TAG, "Failed to create UART event task");
            led_red_seq.blink({.limit_blink_cycles = 4, .fulfill = true});
            return ESP_FAIL;
        }

        return ESP_OK;
    }

    /**
     * @brief Sends the message over UART.
     *
     * @param message
     * @param size
     * @return esp_err_t
     */
    esp_err_t sendMessage(const void* message, size_t size) {
        //writeBytes(tx_cobs_buffer, encoded_size);

        return ESP_OK;
    }

    esp_err_t sendEmergency() override { return true; }
    esp_err_t sendButton(const uint8_t btn_id, const uint16_t duration) override { return true; }
    esp_err_t sendAlive() override { return true; }

   private:
    TaskHandle_t event_task_handle_ = nullptr;
    uint8_t rx_data_buffer[rx_tx_buf_size];  // FIXME: Should better become static members to save heap overflows and fragmentation

    /**
     * @brief UART event task handler required by MainboardAbstractUART.
     *        Get called by UARTs event handler whenever data get received and need to be processed, as quick as possible.
     *
     * @param arg
     */
    static void uart_event_task(void* arg) {
        MainboardDriverYF* caller = (MainboardDriverYF*)arg;  // Caller instance
        uart_event_t event;
        uint8_t* pkt_buf = (uint8_t*)malloc(rx_tx_buf_size);

        for (;;) {
            // Waiting for UART event.
            if (!xQueueReceive(caller->uart_event_queue_, (void*)&event, (TickType_t)portMAX_DELAY))
                continue;

            // FIXME: Is this really necessary? In my point of view it will only waste resources! Disable for now.
            // bzero(pkt_buf, rx_pkt_buf_size);

            // Handle UART events
            switch (event.type) {
                case UART_DATA:
                    uart_read_bytes(caller->port_, pkt_buf, event.size, portMAX_DELAY);
                    ESP_LOGI(TAG, "[UART DATA]: %d bytes", event.size);
                    ESP_LOG_BUFFER_HEXDUMP(TAG, pkt_buf, event.size, ESP_LOG_INFO);

                    // FIXME: The following code should probably go into a separate task with LOWER priority, which might get triggered with a task notification.
                    //        But then we need to take care that we copy the buffer before the next UART event gets processed.
                    caller->onUartReceive(pkt_buf, event.size);
                    break;
                case UART_FIFO_OVF:
                    ESP_LOGW(TAG, "UART_FIFO_OVF");
                    uart_flush_input(caller->port_);
                    xQueueReset(caller->uart_event_queue_);
                    break;
                case UART_BUFFER_FULL:
                    ESP_LOGW(TAG, "Unhandled event UART_BUFFER_FULL");
                    uart_flush_input(caller->port_);
                    xQueueReset(caller->uart_event_queue_);
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

    /**
     * @brief On UART receive get called by uart_event_task once some bytes got received.
     *        It copies the received bytes into a COBS buffer and check for the COBS-end marker.
     *
     * @param rcv_buf
     * @param rcv_size
     */
    void onUartReceive(uint8_t* rcv_buf, size_t rcv_size) {
        //..
    }

};

}  // namespace mainboard_om
