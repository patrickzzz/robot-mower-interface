#include "ButtonState.hpp"
#include <esp_log.h>

namespace YFComms {
    constexpr char TAG[] = "YFComms/ButtonState";

    ButtonState::ButtonState(): observer(nullptr) {
        for (auto& state : buttonStates) {
            state = ButtonStateEnum::LOW;
        }
    }

    ButtonState& ButtonState::setState(Button button, ButtonStateEnum state, uint32_t duration) {
        auto index = static_cast<size_t>(button);
        if (index >= buttonStates.size()) return *this;
        if (buttonStates[index] == state) return *this;

        // Update Zustand und Observer benachrichtigen
        buttonStates[index] = state;

        notifyObserver(button, state, duration);

        return *this;
    }

    ButtonStateEnum ButtonState::getState(Button button) const {
        auto index = static_cast<size_t>(button);
        if (index < buttonStates.size()) {
            return buttonStates[index];
        }
        return ButtonStateEnum::LOW;
    }

    const std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)>& ButtonState::getStates() const {
        return buttonStates;
    }

    ButtonState& ButtonState::setObserver(IButtonEventObserver* observer) {
        this->observer = observer;

        return *this;
    }

    void ButtonState::notifyObserver(Button button, ButtonStateEnum state, uint32_t duration) {
        if (observer != nullptr) {
            observer->onButtonEvent(button, state, duration);
        }
    }
} // namespace YFComms