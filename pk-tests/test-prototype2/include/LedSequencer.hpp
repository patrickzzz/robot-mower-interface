// Created by Jörg Ebeling on 2024-11-30.
// Copyright (c) 2024 Jörg Ebeling for OpenMower/Clemens Elflein and ported to Robot-Interface of patrickzzz. All rights reserved.
//
// This work is licensed under a Creative Commons
// Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// Feel free to use the design in your private/educational projects, but don't
// try to sell the design or products based on it without getting my consent
// first.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <esp_timer.h>

#include "driver/gpio.h"

/**
 * @brief A simple (and light) LED-Sequencer for blink sequences
 *
 * @param pin LED pin of an high-active output GPIO
 */
class LedSequencer {
   public:
    LedSequencer(const gpio_num_t pin) : pin_{pin} {}

    enum class LedMode { OFF,
                         ON,
                         BLINK };

    //  LED blink properties with mostly used blink defaults
    struct SequenceProps {
        LedMode mode = LedMode::BLINK;   // Mode of operation
        uint32_t on_ms = 200;            // On time in ms (only relevant for blink mode)
        uint32_t off_ms = 200;           // Off time in ms (only relevant for blink mode)
        uint8_t limit_blink_cycles = 0;  // Limit blink cycles (only relevant for blink mode)
        uint32_t post_pause = 800;       // Pause after blink sequence (only relevant for blink mode)
        bool fulfill = false;            // Fulfill the complete blink sequence before starting the next one
        bool repeat = false;             // Repeat blink sequence as long as there's no next one

        bool operator==(const SequenceProps& rhs) const = default;
    };

    /**
     * @brief Shorthand for LED-On (once active sequence is done (or !fulfill))
     */
    void on(void) {
        next_({.mode = LedMode::ON, .post_pause = 0});
    }

    /**
     * @brief Shorthand for LED-Off (once active sequence is done (or !fulfill))
     */
    void off(void) {
        next_({.mode = LedMode::OFF, .post_pause = 0});
    }

    /**
     * @brief LED blink sequence with the given blink properties.
     * Get "queued" as next_ sequence, probably overwriting an already existing (but yet not started) next_ sequence.
     *
     * @param props
     */
    void blink(SequenceProps props) {
        props.mode = LedMode::BLINK;
        next_(props);
    }

    /**
     * @brief Generic loop() function for sequence processing.
     * Call as often as possible and as less as required (for a visual LED feedback).
     *
     * @return true if sequence got processed
     * @return false if all sequences got processed
     */
    bool loop(void) {
        if (active_state_.state == State::UNDEF && next_state_.state == State::UNDEF)  // Nothing to do
            return false;

        uint32_t now = esp_timer_get_time() / 1000;

        // Copy next_ to active_ when ...
        if (next_state_.state == State::IDLE &&                                                 // next_ job exists
            (active_state_.state == State::UNDEF ||                                             // active_ finished
             (!active_state_.props.fulfill && !(next_state_.props == active_state_.props)))) {  // active_ has not to be fulfilled and next_ differs to active_
            active_state_ = next_state_;
            next_state_.state = State::UNDEF;
            active_state_.next_state_cycle_millis = now;  // Just activated = next cycle = now
        }

        if (now < active_state_.next_state_cycle_millis)
            return true;

        switch (active_state_.props.mode) {
            case LedMode::ON:
                gpio_set_level(pin_, 1);  // On
                active_state_.state = State::UNDEF;
                break;

            case LedMode::OFF:
                gpio_set_level(pin_, 0);  // Off
                active_state_.state = State::UNDEF;
                break;

            case LedMode::BLINK:
                switch (active_state_.state) {
                    case State::IDLE:
                        active_state_.blink_cycles = 0;
                    /* FALLTHRU */
                    case State::NEXT_BLINK:
                        gpio_set_level(pin_, 1);  // On
                        active_state_.next_state_cycle_millis = now + active_state_.props.on_ms;
                        active_state_.state = State::ON;
                        break;
                    case State::ON:
                        gpio_set_level(pin_, 0);  // Off
                        active_state_.next_state_cycle_millis = now + active_state_.props.off_ms;
                        active_state_.state = State::OFF;
                        break;
                    case State::OFF:  // One complete on/off cycle fulfilled
                        active_state_.blink_cycles++;
                        if (!active_state_.props.limit_blink_cycles || active_state_.blink_cycles < active_state_.props.limit_blink_cycles) {
                            active_state_.state = State::NEXT_BLINK;
                            loop();
                            break;
                        }
                        active_state_.next_state_cycle_millis = now + active_state_.props.post_pause;
                        active_state_.state = State::POST_PAUSE;
                        break;
                    case State::POST_PAUSE:  // Full sequence done
                        if (active_state_.props.repeat && next_state_.state == State::UNDEF) {
                            next_(active_state_.props);
                            break;
                        }
                        active_state_.state = State::UNDEF;
                        break;
                    default:
                        // UNDEF or not yet implemented
                        break;
                }
                break;
        }
        return true;
    }

   private:
    enum class State { UNDEF,
                       IDLE,
                       ON,
                       OFF,
                       NEXT_BLINK,
                       POST_PAUSE };

    struct QueueState {
        SequenceProps props;
        State state = State::UNDEF;        // Current state in sequence (idle, on, off, post_pause, ...)
        uint32_t next_state_cycle_millis;  // Millis when next state-cycle shall happen
        uint8_t blink_cycles;              // Counter for completed blink cycles
    };

    const gpio_num_t pin_;
    QueueState active_state_, next_state_;

    void next_(SequenceProps props) {
        next_state_.props = props;
        next_state_.state = State::IDLE;
        loop();
    }
};