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

namespace mainboards {
constexpr char TAG[] = "mainboards";

/**
 * Mainboard Interface (abstract base class)
 */
class MainboardInterface {
   public:
    /**
     * @brief Construct a new Mainboard Interface object
     *
     * @param on_event_cb get called i.e. for a LED change(d) event, like when receiving a LED-change message via UART or when a LED-change got detected by GPIO read.
     *        Interface/methods for on_event_cb need to be defined.
     */
    MainboardInterface(const void* on_event_cb) : on_event_cb_(on_event_cb) {}
    virtual ~MainboardInterface() {}

    virtual esp_err_t init() = 0;

    // These are the standard message which can be sent to an OM-Mainboard. Does they harmonize with what's needed to be send to a YF-Mainboard?
    virtual esp_err_t sendEmergency() = 0;                                            // Planned to grep the related emergencies out of BHS bitfield. Needs to be reconsidered.
    virtual esp_err_t sendButton(const uint8_t btn_id, const uint16_t duration) = 0;  // OM expects a button message when a button got released. How will it be for YF?
    virtual esp_err_t sendAlive() = 0;                                                // OM uses emergency packet for this, but guess YF has a separate message for this?

   protected:
    const void* on_event_cb_;
};

}  // namespace mainboards
