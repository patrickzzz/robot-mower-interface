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

#include "COBS.h"
#include "CRC.h"
#include "esp_check.h"
#include "esp_log.h"
#include "mainboard_abstract_uart.hpp"
#include "proto_openmower.h"

namespace mainboard_om {
constexpr char TAG[] = "mainboard_om";

constexpr int baud = 115200;
constexpr size_t rx_pkt_buf_size = 100;               // UART receive buffer size
constexpr size_t rx_data_buf_size = rx_pkt_buf_size;  // RX-Data for decoded COBS packet
constexpr size_t tx_cobs_buf_size = rx_pkt_buf_size;  // COBS transmit buffer size

class MainboardDriverOM : public mainboards::MainboardAbstractUART {
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
     * @brief Prepares the message by adding CRC and encoding it with COBS, then sends it over UART.
     *
     * @param message
     * @param size
     * @return esp_err_t
     */
    esp_err_t sendMessage(const void* message, size_t size) {
        // Sanity check of message size
        ESP_RETURN_ON_FALSE(size >= 4, ESP_ERR_INVALID_SIZE, TAG, "Message to small");  // TYPE + >=1 data + 2 byte CRC

        // Prepare pointer to data and CRC
        uint8_t* data_pointer = (uint8_t*)message;
        uint16_t* crc_pointer = (uint16_t*)(data_pointer + (size - 2));

        // Calc CRC
        *crc_pointer = CRC::Calculate(data_pointer, size - 2, CRC::CRC_16_CCITTFALSE());

        // ESP_LOGI(TAG, "Prepared message (%d byte):", size);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, data_pointer, size, ESP_LOG_INFO);

        // COBS encode
        size_t encoded_size = cobs.encode((uint8_t*)message, size, tx_cobs_buffer);
        tx_cobs_buffer[encoded_size] = 0;  // Add COBS-end marker
        encoded_size++;

        // ESP_LOGI(TAG, "COBS encoded message (%d bytes):", encoded_size);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, tx_cobs_buffer, encoded_size, ESP_LOG_INFO);

        writeBytes(tx_cobs_buffer, encoded_size);  // Send COBS encoded message

        return ESP_OK;
    }

    esp_err_t sendEmergency() override { return true; }
    esp_err_t sendButton(const uint8_t btn_id, const uint16_t duration) override { return true; }
    esp_err_t sendAlive() override { return true; }

   private:
    TaskHandle_t event_task_handle_ = nullptr;
    uint8_t rx_data_buffer[rx_data_buf_size];  // FIXME: Should better become static members to save heap overflows and fragmentation
    uint8_t tx_cobs_buffer[tx_cobs_buf_size];  // FIXME: Should better become static members to save heap overflows and fragmentation
    COBS cobs;

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
        static size_t cobs_pos = 0;
        // ESP_LOGI(TAG, "onUartReceive: %d bytes", rcv_size);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, rcv_buf, rcv_size, ESP_LOG_INFO);

        // Process each byte in pkt_buf and extract COBS packet
        for (size_t i = 0; i < rcv_size; ++i) {
            uint8_t readbyte = rcv_buf[i];
            // COBS-end marker
            if (readbyte == 0) {
                if (onCobsPacketReceived(rcv_buf, cobs_pos) != ESP_OK)
                    led_red_seq.blink({.on_ms = 20, .limit_blink_cycles = 1, .fulfill = true});  // Short "comms error" flash
                cobs_pos = 0;
                continue;
            }
            cobs_pos++;
            if (cobs_pos >= rx_data_buf_size) {
                // Buffer full without getting a COBS-end marker. Reset cobs_pos and throw away the rest of this rcv_buffer.
                cobs_pos = 0;
                led_red_seq.blink({.limit_blink_cycles = 1, .fulfill = true});  // Short "comms error" flash
                return;
            }
        }
    }

    /**
     * @brief On COBS packet receive get called by onUartReceive once a COBS encoded packet got received.
     *        It decodes the COBS packet, checks the packet size, packet CRC, and calls onMessageReceiveCB_ (not yet implemented).
     *
     * @param size COBS packet size, without the COBS-end marker
     */
    esp_err_t onCobsPacketReceived(const uint8_t* rcv_buf, const size_t size) {
        // ESP_LOGI(TAG, "onCobsPacketReceived() %d bytes", size);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, rcv_buf, size, ESP_LOG_INFO);

        // Decode COBS packet
        size_t data_size = cobs.decode(rcv_buf, size, rx_data_buffer);
        // ESP_LOGI(TAG, "Decoded COBs packet (%d bytes):", data_size);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, rx_data_buffer, data_size, ESP_LOG_INFO);

        // Sanity check of decoded COBS packet size
        ESP_RETURN_ON_FALSE(size >= 4, ESP_ERR_INVALID_SIZE, TAG, "COBS packet too short");  // TYPE + >=1 data + 2 byte CRC

        // Caluculate CRC
        uint16_t crc = CRC::Calculate(rx_data_buffer, data_size - 2, CRC::CRC_16_CCITTFALSE());
        // ESP_LOGI(TAG, "Calculated crc 0x%x", crc);

        // Switch on received packet TYPE
        switch (rx_data_buffer[0]) {
            // Get_Version packet get send by OM mainboard every 5 seconds to check if a "UI Board" is available.
            // We've to response with a Get_Version packet (and our version) within 100ms to signal that we're alive.
            case TYPE::Get_Version:
                ESP_RETURN_ON_FALSE(data_size == sizeof(struct msg_get_version), ESP_ERR_INVALID_SIZE, TAG, "Invalid 'Get_Version' packet size");
                {
                    struct msg_get_version* message = (struct msg_get_version*)rx_data_buffer;
                    ESP_RETURN_ON_FALSE(crc == message->crc, ESP_ERR_INVALID_CRC, TAG, "Invalid 'Get_Version' packet CRC 0x%x != 0x%x (calculated)", message->crc, crc);
                    ESP_LOGI(TAG, "Received 'Get_Version' packet with (undefined) version 0x%x", message->version);

                    // Respond with our version
                    message->version = firmwareVersion;
                    sendMessage(message, sizeof(struct msg_get_version));
                }
                break;
            case TYPE::Set_LEDs:
                ESP_RETURN_ON_FALSE(data_size == sizeof(struct msg_set_leds), ESP_ERR_INVALID_SIZE, TAG, "Invalid 'Set_LEDs' packet size");
                {
                    struct msg_set_leds* message = (struct msg_set_leds*)rx_data_buffer;
                    ESP_RETURN_ON_FALSE(crc == message->crc, ESP_ERR_INVALID_CRC, TAG, "Invalid 'Set_LEDs' packet CRC 0x%x != 0x%x (calculated)", message->crc, crc);
                    ESP_LOGI(TAG, "Received 'Set_LEDs' packet with leds 0x%llx", message->leds);
                }
                break;
            default:
                ESP_LOGW(TAG, "Unknown or wrong sized packet type 0x%x", rx_data_buffer[0]);
                break;
        }

        return ESP_OK;
    }
};

}  // namespace mainboard_om
