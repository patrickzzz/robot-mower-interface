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
            .flags = {.backup_before_sleep = 0},
        };
        if (esp_err_t ret = uart_param_config(port_, &uart_config) != ESP_OK) {
            ESP_LOGE(TAG, "uart_param_config() failed with error: %s", esp_err_to_name(ret));
            led_red_seq.blink({.limit_blink_cycles = 4, .fulfill = true});
            return ret;
        }
        // Set RX/TX pins
        if (esp_err_t ret = uart_set_pin(port_, tx_pin_, rx_pin_, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
            ESP_LOGE(TAG, "uart_set_pin() failed with error: %s", esp_err_to_name(ret));
            led_red_seq.blink({.limit_blink_cycles = 4, .fulfill = true});
            return ret;
        }
        // Install UART driver
        if (esp_err_t ret = uart_driver_install(port_, rx_buf_size_, tx_buf_size_, 10, &uart_event_queue_, 0) != ESP_OK) {
            ESP_LOGE(TAG, "uart_driver_install() failed with error: %s", esp_err_to_name(ret));
            led_red_seq.blink({.limit_blink_cycles = 4, .fulfill = true});
            return ret;
        }

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
};

}  // namespace mainboards
