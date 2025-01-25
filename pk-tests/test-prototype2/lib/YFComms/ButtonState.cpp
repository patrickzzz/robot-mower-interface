#include "ButtonState.hpp"
#include <esp_log.h>

namespace YFComms {
    constexpr char TAG[] = "YFComms/ButtonState";

    ButtonState::ButtonState() {
        for (auto& state : buttonStates) {
            state = ButtonStateEnum::RELEASED;
        }
    }

    ButtonState& ButtonState::setState(Button button, ButtonStateEnum state, uint32_t duration) {
        auto index = static_cast<size_t>(button);
        if (index >= buttonStates.size()) return *this;
        if (buttonStates[index] == state) return *this;

        // Update Zustand und Observer benachrichtigen
        buttonStates[index] = state;

        notifyObservers(button, state, duration);

        return *this;
    }

    ButtonStateEnum ButtonState::getState(Button button) const {
        auto index = static_cast<size_t>(button);
        if (index < buttonStates.size()) {
            return buttonStates[index];
        }
        return ButtonStateEnum::RELEASED;
    }

    const std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)>& ButtonState::getStates() const {
        return buttonStates;
    }


    void ButtonState::addObserver(IButtonEventObserver* observer) {
        if (std::find(observers.begin(), observers.end(), observer) == observers.end()) {
            observers.push_back(observer);
        }
    }

    void ButtonState::removeObserver(IButtonEventObserver* observer) {
        observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
    }

    void ButtonState::notifyObservers(Button button, ButtonStateEnum state, uint32_t duration) {
        for (auto observer : observers) {
            if (observer) {
                observer->onButtonEvent(button, state, duration);
            }
        }
    }
} // namespace YFComms