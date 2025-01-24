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
