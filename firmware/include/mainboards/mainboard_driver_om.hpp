/* Created by Apehaenger on 2025-01-24.
 * Copyright (c) 2025 Jörg Ebeling. All rights reserved.
 *
 * This file is part of the robot-interface project at https://github.com/patrickzzz/robot-interface.
 *
 * This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * Feel free to use the design in your private/educational projects, but don't try to sell the design or products based on it without getting our consent first.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

#include <stdint.h>

#include "cobs.h" // See https://github.com/PatrickBaus/COBS-CPP
#include "esp_log.h"
#include "mainboard_abstract_uart.hpp"

namespace mainboards {

constexpr int baud = 115200;
constexpr size_t rx_pkt_buf_size = 100;               // UART receive buffer size
constexpr size_t rx_cobs_buf_size = rx_pkt_buf_size;  // COBS receive buffer size

class MainboardDriverOM : public MainboardAbstractUART {
   public:
    MainboardDriverOM(const uart_port_t port, const gpio_num_t rx_pin, const gpio_num_t tx_pin, const void* on_event_cb)
        : MainboardAbstractUART(port, rx_pin, tx_pin, on_event_cb, baud) {}
    ~MainboardDriverOM() {
        // Cleanup FreeRTOS queue
        if (event_task_handle_ != nullptr) {
            vTaskDelete(event_task_handle_);
        }

        // TODO: Free COBS buffer
    }

    esp_err_t init() override {
        ESP_LOGI(TAG, "MainboardDriverOM::init()");

        if (esp_err_t ret = MainboardAbstractUART::init() != ESP_OK) {
            return ret;
        }

        // Create a task to handler UART event from ISR
        xTaskCreate(uart_event_task, "uart_event_task", 4096, (void*)this, 12, &event_task_handle_);
        if (event_task_handle_ == nullptr) {
            ESP_LOGE(TAG, "Failed to create UART event task");
            led_red_seq.blink({.limit_blink_cycles = 4, .fulfill = true});
            return ESP_FAIL;
        }

        // TODO: COBS buffer (PSRAM? instead if heap? but slower)
        // TODO: onCobsPacketReceive()
        return ESP_OK;
    }

    esp_err_t sendEmergency() override { return true; }
    esp_err_t sendButton(const uint8_t btn_id, const uint16_t duration) override { return true; }
    esp_err_t sendAlive() override { return true; }

   private:
    TaskHandle_t event_task_handle_ = nullptr;
    uint8_t cobs_rx_buffer[rx_cobs_buf_size];

    /**
     * @brief UART event task handler required by MainboardAbstractUART.
     *        Get called by UARTs event handler whenever data get received and need to be processed, as quick as possible.
     *
     * @param arg
     */
    static void uart_event_task(void* arg) {
        MainboardDriverOM* caller = (MainboardDriverOM*)arg;  // Caller instance
        uart_event_t event;
        uint8_t* pkt_buf = (uint8_t*)malloc(rx_pkt_buf_size);

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
                    // ESP_LOGI(TAG, "[UART DATA]: %d bytes", event.size);
                    // ESP_LOG_BUFFER_HEXDUMP(TAG, pkt_buf, event.size, ESP_LOG_INFO);

                    // FIXME: The following code should probably go into a separate task with LOWER priority, which might get triggered with a task notification
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
        static size_t cobs_pos = 0;
        // ESP_LOGI(TAG, "onUartReceive: %d bytes", rcv_size);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, rcv_buf, rcv_size, ESP_LOG_INFO);

        // Process each byte in pkt_buf and extract COBS packet
        for (size_t i = 0; i < rcv_size; ++i) {
            uint8_t readbyte = rcv_buf[i];
            cobs_rx_buffer[cobs_pos] = readbyte;
            // COBS-end marker
            if (readbyte == 0) {
                onCobsPacketReceive(cobs_pos);
                cobs_pos = 0;
                continue;
            }
            cobs_pos++;
            if (cobs_pos >= rx_cobs_buf_size) {
                // Buffer full without getting a COBS-end marker. Throw away cons_rx_buffer and reset.
                cobs_pos = 0;
                led_red_seq.blink({.limit_blink_cycles = 1, .fulfill = true});  // Short comms error flash
                continue;
            }
        }
    }

    /**
     * @brief On COBS packet receive get called by onUartReceive once a COBS packet got received.
     *        It decodes the COBS packet, checks the packet crc, and calls onMessageReceiveCB_.
     *
     * @param size
     */
    void onCobsPacketReceive(const size_t size) {
        ESP_LOGI(TAG, "onCobsPacketReceive() %d bytes", size);
        ESP_LOG_BUFFER_HEXDUMP(TAG, cobs_rx_buffer, size, ESP_LOG_INFO);

        cobs::decode(cobs_rx_buffer, size);
        ESP_LOGI(TAG, "Decoded COBs packet:");
        ESP_LOG_BUFFER_HEXDUMP(TAG, cobs_rx_buffer, size, ESP_LOG_INFO);

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
