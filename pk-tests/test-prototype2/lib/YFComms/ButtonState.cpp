#include "ButtonState.hpp"
#include <esp_log.h>

namespace YFComms {
    constexpr char TAG[] = "YFComms/ButtonState";

    ButtonState::ButtonState() {
        for (auto& state : buttonStates) {
            state = ButtonStateEnum::RELEASED;
        }
        for (auto& time : pressStartTimes) {
            time = 0;
        }
    }

    ButtonState& ButtonState::setState(Button button, ButtonStateEnum state, bool isDebounced) {
        auto index = static_cast<size_t>(button);
        if (index >= buttonStates.size()) return *this;

        if (state == ButtonStateEnum::PRESSED && buttonStates[index] == ButtonStateEnum::RELEASED) {
            pressStartTimes[index] = getCurrentTime();
            notifyObservers(button, state, 0);
        } else if (state == ButtonStateEnum::RELEASED && buttonStates[index] == ButtonStateEnum::PRESSED) {
            uint32_t pressDuration = getCurrentTime() - pressStartTimes[index];

            // Debounce: Ignore, if pressDuration is too short (15ms)
            if (!isDebounced && pressDuration < 15) {
                ESP_LOGI(TAG, "Ignored short press (debounced) on button %d", static_cast<uint8_t>(button));
            }else{
                notifyObservers(button, state, pressDuration);
            }
        }

        buttonStates[index] = state;
        isUpdated = true;
        return *this;
    }

    ButtonStateEnum ButtonState::getState(Button button) const {
        auto index = static_cast<size_t>(button);
        if (index < buttonStates.size()) {
            return buttonStates[index];
        }
        return ButtonStateEnum::RELEASED;
    }

    uint32_t ButtonState::getPressDuration(Button button) const {
        auto index = static_cast<size_t>(button);
        if (index < pressStartTimes.size()) {
            return getCurrentTime() - pressStartTimes[index];
        }
        return 0;
    }

    void ButtonState::resetPressDuration(Button button) {
        auto index = static_cast<size_t>(button);
        if (index < pressStartTimes.size()) {
            pressStartTimes[index] = 0;
        }
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