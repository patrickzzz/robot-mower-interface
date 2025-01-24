#include "LEDState.hpp"
#include "esp_log.h"

namespace YFComms {
    constexpr char TAG[] = "YFComms/LEDState";

    LEDState::LEDState() {
        for (LEDStateEnum& state : ledStates) {
            state = LEDStateEnum::OFF;
        }
    }

    LEDState& LEDState::setState(LED led, LEDStateEnum state) {
        if (static_cast<size_t>(led) < ledStates.size()) {
            ledStates[static_cast<size_t>(led)] = state;

            isUpdated = true;
        }

        return *this;
    }

    LEDStateEnum LEDState::getState(LED led) const {
        if (static_cast<size_t>(led) < ledStates.size()) {
            return ledStates[static_cast<size_t>(led)];
        }
        return LEDStateEnum::OFF;
    }

    const std::array<LEDStateEnum, static_cast<size_t>(LED::MAX)>& LEDState::getStates() const {
        return ledStates;
    }

    void LEDState::setStates(LEDStateEnum state) {
        for (auto& ledState : ledStates) {
            ledState = state;
        }
    }
} // namespace YFComms