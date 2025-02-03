#include "ButtonState.hpp"
#include <esp_log.h>

namespace YFComms {
    constexpr char TAG[] = "YFComms/ButtonState";

    ButtonState::ButtonState(): listener(nullptr) {
        for (auto& state : buttonStates) {
            state = ButtonStateEnum::LOW;
        }
    }

    ButtonState& ButtonState::setState(Button button, ButtonStateEnum state, uint32_t duration) {
        auto index = static_cast<size_t>(button);
        if (index >= buttonStates.size()) return *this;
        if (buttonStates[index] == state) return *this;

        // Update Zustand und Listener benachrichtigen
        buttonStates[index] = state;

        notifyListener(button, state, duration);

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

    ButtonState& ButtonState::setListener(AbstractButtonEventListener* listener) {
        this->listener = listener;

        return *this;
    }

    void ButtonState::notifyListener(Button button, ButtonStateEnum state, uint32_t duration) {
        if (listener != nullptr) {
            listener->onButtonEvent(button, state, duration);
        }
    }
} // namespace YFComms