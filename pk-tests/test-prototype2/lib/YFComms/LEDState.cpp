#include "LEDState.hpp"
#include "esp_log.h"

namespace YFComms {
    constexpr char TAG[] = "YFComms/LEDState";

    LEDState::LEDState() {
        for (LEDStateEnum& state : ledStates) {
            state = LEDStateEnum::OFF;
        }
    }

    void LEDState::setLEDState(LED led, LEDStateEnum state) {
        if (static_cast<size_t>(led) < ledStates.size()) {
            ledStates[static_cast<size_t>(led)] = state;
        }
    }

    LEDStateEnum LEDState::getLEDState(LED led) const {
        if (static_cast<size_t>(led) < ledStates.size()) {
            return ledStates[static_cast<size_t>(led)];
        }
        return LEDStateEnum::OFF;
    }

    const std::array<LEDStateEnum, static_cast<size_t>(LED::MAX)>& LEDState::getLEDStates() const {
        return ledStates;
    }

    void LEDState::setLEDStates(LEDStateEnum state) {
        for (auto& ledState : ledStates) {
            ledState = state;
        }
    }

    void LEDState::printStates() const {
        for (size_t i = 0; i < ledStates.size(); ++i) {
            ESP_LOGI("LEDState", "LED %zu: %s", i, stateToString(ledStates[i]));
        }
    }

    const char* LEDState::stateToString(LEDStateEnum state) {
        switch (state) {
            case LEDStateEnum::OFF: return "OFF";
            case LEDStateEnum::ON: return "ON";
            case LEDStateEnum::FLASH_SLOW: return "FLASH_SLOW";
            case LEDStateEnum::FLASH_FAST: return "FLASH_FAST";
            default: return "UNKNOWN";
        }
    }
} // namespace YFComms