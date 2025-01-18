#pragma once

#include <freertos/FreeRTOS.h>

constexpr TickType_t ledsTaskCycle = 10;  // How long to suspend the LEDs-Task in ms

/**
 * @brief SystemHealth bitfield is for indicating if every system specific is healthy.
 * This is mainly for reaching the backend for a status what's up with the system
 * and not blindly blink with the red LED in a closed mower.
 */
struct SystemHealth {
    bool port_expander_i2c : 1;  // I2C for port expander okay
    bool port_expander : 1;      // Port expander TCA9555 okay
};