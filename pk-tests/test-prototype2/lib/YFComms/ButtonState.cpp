#include "ButtonState.hpp"
#include <esp_log.h>

namespace YFComms {
    constexpr char TAG[] = "YFComms/ButtonState";

    ButtonState::ButtonState() {
        for (auto& state : buttonStates) {
            state = ButtonStateEnum::RELEASED;
        }
    }

    ButtonState& ButtonState::setState(Button button, ButtonStateEnum state) {
        if (static_cast<size_t>(button) < buttonStates.size()) {
            buttonStates[static_cast<size_t>(button)] = state;
            isUpdated = true;
        }
        return *this;
    }

    ButtonStateEnum ButtonState::getState(Button button) const {
        if (static_cast<size_t>(button) < buttonStates.size()) {
            return buttonStates[static_cast<size_t>(button)];
        }
        return ButtonStateEnum::RELEASED;
    }

    const std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)>& ButtonState::getStates() const {
        return buttonStates;
    }
} // namespace YFComms