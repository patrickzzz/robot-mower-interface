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

#include "esp_log.h"
#include "mainboard_interface.hpp"

namespace mainboards {

/**
 * Generic UART Mainboard Interface (abstract sub class)
 */
class MainboardAbstractUART : public MainboardInterface {
   public:
    /**
     * @brief Construct a new UART specific MainboardInterface
     *
     * @param port UART to use (we only have UART_NUM_1 and UART_NUM_2 free for use)
     * @param rx_pin GPIO pin
     * @param tx_pin GPIO pin
     * @param on_event_cb get called i.e. for a LED change(d) event, like when receiving a LED-change message via UART.
     *        Interface/methods for on_event_cb need to be defined.
     * @param baud optional default 115200
     * @param rx_buf_size of ringbuffer. Optional, default 200 FIXME: 64 byte should be enough. Test when having enough data or testsuite ready
     * @param tx_buf_size of ringbuffer. Optional, default 200 FIXME: 64 byte should be enough. Test when having enough data or testsuite ready
     */
    MainboardAbstractUART(const uart_port_t port, const gpio_num_t rx_pin, const gpio_num_t tx_pin, const void* on_event_cb, const int baud = 115200, const int rx_buf_size = 200, const int tx_buf_size = 200)
        : MainboardInterface(on_event_cb), port_(port), rx_pin_(rx_pin), tx_pin_(tx_pin), baud_(baud), rx_buf_size_(rx_buf_size), tx_buf_size_(tx_buf_size) {}

    ~MainboardAbstractUART() {
        // UART driver cleanup
        ESP_ERROR_CHECK_WITHOUT_ABORT(uart_driver_delete(port_));
    }

    /**
     * @brief Initialize UART
     *
     * @return esp_err_t
     */
    esp_err_t init() override {
        ESP_LOGI(TAG, "MainboardAbstractUART::init()");

        // Configure UART
        const uart_config_t uart_config = {
            .baud_rate = baud_,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 0,
            .source_clk = UART_SCLK_DEFAULT,
            .flags = {.allow_pd = 0, .backup_before_sleep = 0},
        };
        ESP_RETURN_ON_ERROR(uart_param_config(port_, &uart_config), TAG, "uart_param_config() failed");

        // Set RX/TX pins
        ESP_RETURN_ON_ERROR(uart_set_pin(port_, tx_pin_, rx_pin_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE), TAG, "uart_set_pin() failed");

        // Install UART driver
        ESP_RETURN_ON_ERROR(uart_driver_install(port_, rx_buf_size_, tx_buf_size_, 10, &uart_event_queue_, 0), TAG, "uart_driver_install() failed");

        return ESP_OK;
    }

   protected:
    const uart_port_t port_;
    const gpio_num_t rx_pin_;
    const gpio_num_t tx_pin_;
    const int baud_;
    const int rx_buf_size_;
    const int tx_buf_size_;

    QueueHandle_t uart_event_queue_;

    /**
     * @brief Write bytes to the UART- TX ring buffer (either immediately or after enough space is available), and then exit.
     *        So it will block as long as there's not enough space in the TX ring buffer.
     *        When there is free space in the TX FIFO buffer, an interrupt service routine (ISR) moves the data from the TX ring buffer to the TX FIFO buffer in the background
     *
     * @param buffer
     * @param size
     * @return esp_err_t
     */
    size_t writeBytes(const uint8_t* buffer, size_t size) {
        // ESP_LOGI(TAG, "writeBytes(): %d bytes", size);
        // ESP_LOG_BUFFER_HEXDUMP(TAG, buffer, size, ESP_LOG_INFO);

        return uart_write_bytes(port_, (const char*)buffer, size);
    }
};
}  // namespace mainboards
