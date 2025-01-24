#pragma once

#include <stdint.h>

#include "esp_log.h"
#include "mainboard_abstract_uart.hpp"

namespace mainboards {

constexpr int baud = 115200;

static void uart_event_task(void* pvParameters) {
    uart_event_t event;
    //size_t buffered_size;
    uint8_t* dtmp = (uint8_t*)malloc(200);
    for (;;) {
        // Waiting for UART event.
        if (xQueueReceive(queue_, (void*)&event, (TickType_t)portMAX_DELAY)) {
            bzero(dtmp, 200);
            ESP_LOGI(TAG, "uart[%d] event: %d", UART_NUM_2, event.type);
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

class MainboardDriverOM : public MainboardAbstractUART {
   public:
    MainboardDriverOM(const uart_port_t port, const gpio_num_t rx_pin, const gpio_num_t tx_pin, const void* on_event_cb) 
    : MainboardAbstractUART(port, rx_pin, tx_pin, uart_event_task, on_event_cb, baud) {}
    ~MainboardDriverOM() {
        // TODO: Free COBS buffer
        // TODO: Free Packet buffer
    }

    esp_err_t init() override {
        ESP_LOGI(TAG, "MainboardDriverOM::init()");
        esp_err_t ret = MainboardAbstractUART::init();
        // TODO: COBS buffer (PSRAM? instead if heap? but slower)
        // TODO: Packet buffer (PSRAM? instead if heap? but slower)
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
