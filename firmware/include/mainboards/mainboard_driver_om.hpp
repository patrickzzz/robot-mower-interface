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

#include "esp_log.h"
#include "mainboard_abstract_uart.hpp"

namespace mainboards {

constexpr int baud = 115200;

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

    /**
     * @brief UART event task handler required by MainboardAbstractUART.
     *        Get called by UARTs event handler whenever data get received and need to be processed, as quick as possible.
     *
     * @param arg
     */
    static void uart_event_task(void* arg) {
        constexpr size_t rx_pkt_buf_size = 200;               // FIXME: 100 byte should be enough. Test when having enough data or testsuite ready
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
                    ESP_LOGI(TAG, "[UART DATA]: %d bytes", event.size);
                    uart_read_bytes(caller->port_, pkt_buf, event.size, portMAX_DELAY);
                    ESP_LOG_BUFFER_HEXDUMP(TAG, pkt_buf, event.size, ESP_LOG_INFO);
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
