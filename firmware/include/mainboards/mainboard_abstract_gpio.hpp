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

#include "mainboard_interface.hpp"

namespace mainboards {

/**
 * Generic GPIO Mainboard Interface
 */
class MainboardAbstractGPIO : public MainboardInterface {
   public:
    /**
     * @brief Construct a new GPIO specific MainboardInterface
     *
     * @param on_event_cb get called i.e. when a LED-change got detected by GPIO read.
     *        Interface/methods for on_event_cb need to be defined.
     *
     * Needs to be defined.
     */
    MainboardAbstractGPIO(const void* on_event_cb) : MainboardInterface(on_event_cb) {}
    ~MainboardAbstractGPIO() {}

    bool init() override { return false; }
};

}  // namespace mainboards
