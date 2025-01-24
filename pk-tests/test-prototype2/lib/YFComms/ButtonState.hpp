#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <esp_timer.h>
#include <vector>
#include <algorithm>
#include "IButtonEventObserver.hpp"

namespace YFComms {
    enum class Button : uint8_t {
        PLAY,
        HOME,
        LOCK,
        LIFT,
        S1,
        S2,
        HOURS_TWO,
        HOURS_FOUR,
        HOURS_SIX,
        HOURS_EIGHT,
        HOURS_TEN,
        DAYS_MON,
        DAYS_TUE,
        DAYS_WED,
        DAYS_THR,
        DAYS_FRI,
        DAYS_SAT,
        DAYS_SUN,
        MAX
    };

    enum class ButtonStateEnum : uint8_t {
        RELEASED = 0x00,
        PRESSED = 0x01
    };

    class ButtonState {
    public:
        ButtonState();

        ButtonState& setState(Button button, ButtonStateEnum state, bool isDebounced = false);
        ButtonStateEnum getState(Button button) const;

        uint32_t getPressDuration(Button button) const; // Press-Dauer abrufen
        void resetPressDuration(Button button);         // Reset der Dauer

        const std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)>& getStates() const;
        bool getIsUpdated() const { return isUpdated; }
        void setIsUpdated(bool updated) { isUpdated = updated; }

        void addObserver(IButtonEventObserver* observer);
        void removeObserver(IButtonEventObserver* observer);

    private:
        bool isUpdated = false;
        std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)> buttonStates;
        std::array<uint32_t, static_cast<size_t>(Button::MAX)> pressStartTimes;
        uint32_t getCurrentTime() const {
            return static_cast<uint32_t>(esp_timer_get_time() / 1000);
        }
        std::vector<IButtonEventObserver*> observers;
        void notifyObservers(Button button, ButtonStateEnum state, uint32_t duration);
    };
} // namespace YFComms