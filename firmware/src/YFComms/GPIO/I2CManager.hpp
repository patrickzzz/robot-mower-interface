#pragma once

#include "esp_io_expander.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "esp_io_expander_tca95xx_16bit/include/esp_io_expander_tca95xx_16bit.h"

class I2CManager {
public:
    static I2CManager& getInstance() {
        static I2CManager instance;
        return instance;
    }

    esp_io_expander_handle_t getIoExpander() {
        if (ioExpander == nullptr) {
            initializeExpander();
        }
        return ioExpander;
    }

    ~I2CManager() {
        if (ioExpander) {
            esp_io_expander_reset(ioExpander);
            ioExpander = nullptr;
        }
        if (i2cHandle) {
            i2c_del_master_bus(i2cHandle);
            i2cHandle = nullptr;
        }
    }

private:
    I2CManager() : i2cHandle(nullptr), ioExpander(nullptr) {}
    I2CManager(const I2CManager&) = delete;
    I2CManager& operator=(const I2CManager&) = delete;

    void initializeExpander() {
        if (i2cHandle == nullptr) {
            const i2c_master_bus_config_t busConfig = {
                .i2c_port = I2C_NUM_0,
                .sda_io_num = GPIO_NUM_35,
                .scl_io_num = GPIO_NUM_36,
                .clk_source = I2C_CLK_SRC_DEFAULT,
                .glitch_ignore_cnt = 7,
                .intr_priority = 0,
                .trans_queue_depth = 0,
                .flags = {.enable_internal_pullup = 1, .allow_pd = 0},
            };

            esp_err_t ret = i2c_new_master_bus(&busConfig, &i2cHandle);
            if (ret != ESP_OK) {
                ESP_LOGE("I2CManager", "Failed to initialize I2C bus: %s", esp_err_to_name(ret));
                return;
            }
        }

        if (ioExpander == nullptr) {
            esp_err_t ret = esp_io_expander_new_i2c_tca95xx_16bit(
                i2cHandle, ESP_IO_EXPANDER_I2C_TCA9555_ADDRESS_000, &ioExpander);
            if (ret != ESP_OK) {
                ESP_LOGE("I2CManager", "Failed to initialize I/O expander: %s", esp_err_to_name(ret));
                return;
            }
            ESP_LOGI("I2CManager", "I/O Expander initialized successfully.");

            if (esp_err_t ret = esp_io_expander_set_dir(ioExpander, IO_EXPANDER_PIN_NUM_0 | IO_EXPANDER_PIN_NUM_1 | IO_EXPANDER_PIN_NUM_2 |
    IO_EXPANDER_PIN_NUM_3 | IO_EXPANDER_PIN_NUM_4 | IO_EXPANDER_PIN_NUM_5 |
    IO_EXPANDER_PIN_NUM_6 | IO_EXPANDER_PIN_NUM_7 | IO_EXPANDER_PIN_NUM_8 |
    IO_EXPANDER_PIN_NUM_9 | IO_EXPANDER_PIN_NUM_10 | IO_EXPANDER_PIN_NUM_11 |
    IO_EXPANDER_PIN_NUM_12 | IO_EXPANDER_PIN_NUM_13 | IO_EXPANDER_PIN_NUM_14, IO_EXPANDER_INPUT) != ESP_OK) {
                ESP_LOGW("I2CManager", "Port expander 'set direction' failed with error: %s", esp_err_to_name(ret));
                return;
            }
 /*
            if (esp_err_t ret = esp_io_expander_set_dir(ioExpander, IO_EXPANDER_PIN_NUM_12, IO_EXPANDER_INPUT) != ESP_OK) {
                ESP_LOGW("I2CManager", "Port expander 'set direction' failed with error: %s", esp_err_to_name(ret));
                return;
               }

  */
        }
    }

    i2c_master_bus_handle_t i2cHandle;
    esp_io_expander_handle_t ioExpander;
};