/* Created by Apehaenger on 2025-02-06.
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

#include <array>
#include <cstdint>
#include <cstdio>

#include "BoardConfig/AbstractBoardConfig.hpp"

namespace YFComms {
namespace led {  // namespaces should be lower case

// Fixed LED names
enum class Names {
    LIFTED,
    SIGNAL,
    BATTERY_LOW,
    CHARGING,
    S1,
    S2,
    LOCK,
    WIFI,
    GPS,
    BACKGROUND,
    HOURS_TWO,
    HOURS_FOUR,
    HOURS_SIX,
    HOURS_EIGHT,
    HOURS_TEN,
    DAY_MON,
    DAY_TUE,
    DAY_WED,
    DAY_THR,
    DAY_FRI,
    DAY_SAT,
    DAY_SUN,
    MAX
};

// Possible led modes
enum class Modes {
    OFF = 0x00,
    ON = 0x10,
    FLASH_SLOW = 0x20,
    FLASH_FAST = 0x22
};

class LED {
   public:
    LED(const YFComms::AbstractBoardConfig::CommunicationType comm_type, const gpio_num_t gpio_pin, const uint8_t uart_msg_pos)
        : comm_type_(comm_type), pin_(gpio_pin), uart_msg_pos_(uart_msg_pos) {};  // Constructor

    /**
     * @brief Set the mode of the LED and return if it changed
     *
     * @param mode
     * @return true
     * @return false
     */
    bool setMode(const Modes mode) {
        auto last_mode = mode_;
        mode_ = mode;  // Set the new mode
        return last_mode != mode_;
    }

    // Get mode of the LED
    Modes getMode() const { return mode_; }

    AbstractBoardConfig::CommunicationType getCommType() const { return comm_type_; }

    // TODO: This should go into a derived class, as well as the used vars, if UART doesn't use this. But for for easier understanding I left it here
    void gpioSetLevel(int level) {
        if (level == level_)
            return;  // No need to spam GPIOs

        level_ = level;
        gpio_set_level(pin_, level);
    }

    // TODO: This should go into a derived class, as well as the used vars, if UART doesn't use this. But for for easier understanding I left it here
    void gpioToggle() {
        level_ = !level_;
        gpio_set_level(pin_, level_);
    }

   private:
    Modes mode_ = Modes::OFF;
    int level_ = 0;  // The current LED (GPIO) level (0 or 1)
    AbstractBoardConfig::CommunicationType comm_type_;
    gpio_num_t pin_;
    uint8_t uart_msg_pos_;
};
}  // namespace led
}  // namespace YFComms