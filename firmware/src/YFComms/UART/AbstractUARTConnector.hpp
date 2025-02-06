#pragma once

#include <driver/uart.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace YFComms {

class AbstractUARTConnector {
public:
    AbstractUARTConnector(uart_port_t uartPort, gpio_num_t rxPin, gpio_num_t txPin, int baudRate = 115200)
        : uartPort(uartPort), rxPin(rxPin), txPin(txPin), baudRate(baudRate), uart_event_queue(nullptr) {}

    virtual ~AbstractUARTConnector() {
        if (uart_event_queue != nullptr) {
            vQueueDelete(uart_event_queue);
            uart_event_queue = nullptr;
        }
        uart_driver_delete(uartPort);
    }

    esp_err_t init() {
        uart_config_t uart_config = {
            .baud_rate = baudRate,
            .data_bits = UART_DATA_8_BITS,
            .parity    = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 0,
            .source_clk = UART_SCLK_APB,
            //.flags = {.allow_pd = 0, .backup_before_sleep = 0},
            .flags = {.backup_before_sleep = 0},
        };

        ESP_ERROR_CHECK(uart_param_config(uartPort, &uart_config));
        ESP_ERROR_CHECK(uart_set_pin(uartPort, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
        ESP_ERROR_CHECK(uart_driver_install(uartPort, 1024, 1024, 10, &uart_event_queue, 0));

        std::string taskName = "uart_event_task_" + std::to_string(static_cast<int>(uartPort));

        xTaskCreate(uart_event_task, taskName.c_str(), 4096, this, 12, &uartTaskHandle);

        // log message, that UART is initialized on port, txpin, rxpin and baudrate
        ESP_LOGI(TAG, "UART initialized on port %d, txPin %d, rxPin %d, baudRate %d", uartPort, txPin, rxPin, baudRate);

        return ESP_OK;
    }

    esp_err_t sendMessage(const uint8_t* data, size_t length) {
        /*
        // Format the message as hex string
        char hexMessage[length * 3 + 1]; // 2 Zeichen pro Byte + Leerzeichen + Nullterminator
        int pos = 0;
        for (int i = 0; i < length; i++) {
            pos += snprintf(&hexMessage[pos], sizeof(hexMessage) - pos, "%02X ", message[i]);
        }
        ESP_LOGI("UART SEND", "%s", hexMessage);
        */

        if(uart_write_bytes(uartPort, reinterpret_cast<const char*>(data), length) != length) {
            ESP_LOGE(TAG, "Failed to send message");
            return ESP_FAIL;
        }

        return ESP_OK;
    }

protected:
    static constexpr char TAG[] = "YFComms/UART";
    static constexpr int MAX_MESSAGE_LENGTH = 128;
    virtual void onUARTReceive(const uint8_t* data, size_t length) = 0;
    TaskHandle_t uartTaskHandle = nullptr;

private:
    uart_port_t uartPort;
    gpio_num_t rxPin;
    gpio_num_t txPin;
    int baudRate;
    QueueHandle_t uart_event_queue;

    static void uart_event_task(void* pvParameters) {
        auto* instance = static_cast<AbstractUARTConnector*>(pvParameters);
        uart_event_t event;
        uint8_t data[MAX_MESSAGE_LENGTH];
        size_t readSize;

        for (;;) {
            // Waiting for UART event.
            if (!xQueueReceive(instance->uart_event_queue, (void*)&event, (TickType_t)portMAX_DELAY))
                continue;

            // Handle UART events
            switch (event.type) {
                case UART_DATA:
                    ESP_LOGI(TAG, "UART_DATA event on port %d, Data length: %d", instance->uartPort, event.size);

                    readSize = uart_read_bytes(instance->uartPort, data, 64, 10 / portTICK_PERIOD_MS);
                    instance->onUARTReceive(data, readSize);
                    break;
                case UART_FIFO_OVF:
                    ESP_LOGW(TAG, "UART_FIFO_OVF");
                    uart_flush_input(instance->uartPort);
                    xQueueReset(instance->uart_event_queue);
                    break;
                case UART_BUFFER_FULL:
                    ESP_LOGW(TAG, "Unhandled event UART_BUFFER_FULL");
                    uart_flush_input(instance->uartPort);
                    xQueueReset(instance->uart_event_queue);
                    break;
                default:
                    ESP_LOGW(TAG, "Unknown/Unhandled event type: %d", event.type);
                    break;
            }
        }
        vTaskDelete(NULL);
    }
};

}  // namespace YFComms