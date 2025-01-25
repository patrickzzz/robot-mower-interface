/* Created by Apehaenger on 2024-12-02.
 * Copyright (c) 2024 Jörg Ebeling. All rights reserved.
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

#include <esp_log.h>
#include <freertos/queue.h>
#include <inttypes.h>

#include "config.h"
#include "esp_io_expander_tca95xx_16bit/include/esp_io_expander_tca95xx_16bit.h"
#include "I2CManager.hpp"
#include "pins.h"

extern LedSequencer led_red_seq;

/**
 * @brief Button-Hall-Switch Sensors, handle all of those input sensors like:
 * - User Button on the CoverUI
 * - Hall Switches connected to the CoverUI
 * - Normal switches like Stop-Switches on the CoverUI or the Adapter-PCB
 *
 * All input signals get debounced before processed!
 */
namespace bhs_sensors {

// For efficiency and simplicity port expanders input value get assigned 1:1 to a bitfields struct (+ the mapped GPIO play button)
struct State {
#ifdef HW_0_1_X
    bool stop_1 : 1;        // 2 pin plug @ CoverUI. Fit with back-handle plug of one side
    bool stop_2 : 1;        // 2 pin plug @ CoverUI. Fit with back-handle plug of the other side
    bool lift : 1;          // Hall (lift) sensor
    bool lift_x : 1;        // Hall (lift2) sensor
    bool bump_l : 1;        // Hall (bumper) sensor
    bool bump_r : 1;        // Hall (bumper2) sensor
    bool btn_home : 1;      // CoverUI button
    bool shell_stop_1 : 1;  // Shell Stop Plug J8 (pin 1-2) on adapter PCB
    bool btn_lock : 1;      // CoverUI button
    bool btn_s1 : 1;        // CoverUI button
    bool btn_s2 : 1;        // CoverUI button
    // Same as with the "hr" LEDs. If a CoverUI has "hr" buttons, it has 4 of them, but their value is model specific
    bool btn_hr_14 : 1;     // CoverUI button
    bool btn_hr_24 : 1;     // CoverUI button
    bool btn_hr_34 : 1;     // CoverUI button
    bool btn_hr_44 : 1;     // CoverUI button
    bool shell_stop_2 : 1;  // Shell Stop Plug J8 (pin 2-3) on adapter PCB
    // GPIO button inputs get MSB prefixed to TCA9555 inputs
    bool btn_play : 1;  // CoverUI button
#elif HW_0_2_X
    bool stop_2 : 1;        // 2 pin plug @ CoverUI. Fit with back-handle plug of the other side
    bool lift_x : 1;        // Hall (lift2) sensor
    bool bump_l : 1;        // Hall (bumper) sensor
    bool stop_1 : 1;        // 2 pin plug @ CoverUI. Fit with back-handle plug of one side
    bool bump_r : 1;        // Hall (bumper2) sensor
    bool lift : 1;          // Hall (lift) sensor
    bool shell_stop_1 : 1;  // Shell Stop Plug J8 (pin 1-2) on adapter PCB
    bool shell_stop_2 : 1;  // Shell Stop Plug J8 (pin 2-3) on adapter PCB
    // Same as with the "hr" LEDs. If a CoverUI has "hr" buttons, it has 4 of them, but their value is model specific
    bool btn_hr_14 : 1;   // CoverUI button
    bool btn_hr_24 : 1;   // CoverUI button
    bool btn_hr_34 : 1;   // CoverUI button
    bool btn_hr_44 : 1;   // CoverUI button
    bool btn_lock : 1;    // CoverUI button
    bool btn_s2 : 1;      // CoverUI button
    bool btn_s1 : 1;      // CoverUI button
    bool btn_unused : 1;  // Unused TCA input
    // GPIO button inputs get MSB prefixed to TCA9555 inputs
    bool btn_play : 1;  // CoverUI button
    bool btn_home : 1;  // CoverUI button
#else
#error No hardware version HW_x_y_z defined
#endif
};

const char *TAG = "bhs_sensors";

// A typical bounce time is between 2ms up to 10ms.
// To be on the save side, the result of taskCycle * numDebounceStates should be larger
constexpr TickType_t taskCycle = 5;
constexpr uint8_t numDebounceStates = 6;

namespace {  // Anonymous private namespace

// Port expander vars
//static i2c_master_bus_handle_t i2c_handle_ = NULL;
static esp_io_expander_handle_t io_expander_ = NULL;

// BHS states
union StateU {
    uint32_t raw;
    State state;
};
static StateU state_debounced_;  // Current state of debounced BHS sensors

}  // namespace

/**
 * @brief Initialize GPIOs and I2C interfaces
 *
 * @return esp_err_t
 */
esp_err_t init(SystemHealth &sys_health) {
    ESP_LOGI(TAG, "init()");

/*
    // Init I2C for TCA9555 port expander
    const i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = portExpSDA,
        .scl_io_num = portExpSCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {.enable_internal_pullup = 1}};
    if (esp_err_t ret = i2c_new_master_bus(&bus_config, &i2c_handle_) != ESP_OK) {
        ESP_LOGW(TAG, "I2C init failed with error: %s", esp_err_to_name(ret));
        return ret;
    }
    sys_health.port_expander_i2c = true;

    // Init TCA9555 port expander
    if (esp_err_t ret = esp_io_expander_new_i2c_tca95xx_16bit(i2c_handle_, ESP_IO_EXPANDER_I2C_TCA9555_ADDRESS_000, &io_expander_) != ESP_OK) {
        ESP_LOGW(TAG, "Port expander connect failed with error: %s", esp_err_to_name(ret));
        return ret;
    }

    if (esp_err_t ret = esp_io_expander_set_dir(io_expander_, portExpBtnPinMask, IO_EXPANDER_INPUT) != ESP_OK) {
        ESP_LOGW(TAG, "Port expander 'set direction' failed with error: %s", esp_err_to_name(ret));
        return ret;
    }

 */
    io_expander_ = I2CManager::getInstance().getIoExpander();
    sys_health.port_expander = true;

    esp_io_expander_print_state(io_expander_);

    // Play button on normal GPIO
    gpio_config_t gpio_conf = {
        .pin_bit_mask = (1ULL << pinButtonPlay),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&gpio_conf);

    return ESP_OK;
}

/**
 * @brief Return (debounced) State bitfield for any button, hall or switch.
 * @return State
 */
State getState() {
    return state_debounced_.state;
}

/**
 * @brief Has to get called regulary i.e. by timer or task (approx. 5ms).
 * Store the (button, hall and switch) GPIO states within states_ array and debounces em.
 */
void task(void *arg) {
    static uint32_t states_[numDebounceStates];  // GPIO port state recorder for every taskCycle time interval
    static uint8_t state_index_ = 0;             // Index for next states_ store positions
    // uint32_t state_changed_;                  // Just changed buttons. Might be used if button events are required

    while (1) {
        uint32_t val = 0;
        // Get port expander buttons
        if (esp_err_t ret = esp_io_expander_get_level(io_expander_, portExpBtnPinMask, &val) == ESP_OK) {
            states_[state_index_] = val ^ 0xffff;  // XOR for active-low buttons
        } else {
            led_red_seq.blink({.on_ms = 100, .off_ms = 100, .limit_blink_cycles = 1, .fulfill = true});  // One quick red flash indicate a communication error;
            ESP_LOGW(TAG, "Port expander read failed with error: %s", esp_err_to_name(ret));
        }

        // Add mergeable button(s)
#ifdef HW_0_1_X
        states_[state_index_] |= (gpio_get_level(pinButtonPlay) ? 0 : (uint32_t)mergedExpPinBtnPlay);
#elif HW_0_2_X
        states_[state_index_] |= (gpio_get_level(pinButtonPlay) ? 0 : (uint32_t)mergedExpPinBtnPlay) | (gpio_get_level(pinButtonHome) ? 0 : (uint32_t)mergedExpPinBtnHome);
#else
#error No hardware version HW_x_y_z defined
#endif

        // Debounce
        // uint32_t laststate_debounced_ = state_debounced_.raw;
        unsigned int i;
        for (i = 0, state_debounced_.raw = 0xffffffff; i < numDebounceStates; i++)
            state_debounced_.raw &= states_[i];

        /* AH20241204 Could be used for BHS event trigger of changed BHS sensors
        // Save what changed
        state_changed_ = state_debounced_ ^ laststate_debounced_.raw; */

        // Circular buffer index
        state_index_++;
        if (state_index_ >= numDebounceStates)
            state_index_ = 0;

        vTaskDelay(taskCycle / portTICK_PERIOD_MS);
    }
}

}  // namespace bhs_sensors