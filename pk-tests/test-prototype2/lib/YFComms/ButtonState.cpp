#include "ButtonState.hpp"

namespace YFComms {
    ButtonState::ButtonState() {
        // Initialize all buttons to released
        for (ButtonStateEnum& state : buttonStates) {
            state = ButtonStateEnum::RELEASED;
        }
    }

    void ButtonState::setButtonState(Button button, ButtonStateEnum state) {
        if (static_cast<size_t>(button) < buttonStates.size()) {
            buttonStates[static_cast<size_t>(button)] = state;
        }
    }

    ButtonStateEnum ButtonState::getButtonState(Button button) const {
        if (static_cast<size_t>(button) < buttonStates.size()) {
            return buttonStates[static_cast<size_t>(button)];
        }
        return ButtonStateEnum::RELEASED;
    }

    const std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)>& ButtonState::getButtonStates() const {
        return buttonStates;
    }

    void ButtonState::setAllButtonStates(ButtonStateEnum state) {
        for (auto& buttonState : buttonStates) {
            buttonState = state;
        }
    }

    void ButtonState::printButtonStates() const {
        for (size_t i = 0; i < buttonStates.size(); ++i) {
            printf("Button %zu: %s\n", i, stateToString(buttonStates[i]));
        }
    }

    const char* ButtonState::stateToString(ButtonStateEnum state) {
        switch (state) {
            case ButtonStateEnum::RELEASED: return "RELEASED";
            case ButtonStateEnum::PRESSED: return "PRESSED";
            default: return "UNKNOWN";
        }
    }
} // namespace YFComms