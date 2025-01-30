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
        // Buttons
        PLAY,
        HOME,
        LOCK,
        OK,
        S1,
        S2,
        CLOCK,
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

        // Halls
        STOP1,
        STOP2,
        LIFT,
        LIFTX,
        BUMPL,
        BUMPR,

        // Shell Stop
        SHELL_STOP1,
        SHELL_STOP2,
        // ..
        MAX
    };

    enum class ButtonStateEnum : uint8_t {
        LOW = 0x00,
        HIGH = 0x01
    };

    class ButtonState {
    public:
        ButtonState();

        ButtonState& setState(Button button, ButtonStateEnum state, uint32_t duration);
        ButtonStateEnum getState(Button button) const;

        const std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)>& getStates() const;

        void addObserver(IButtonEventObserver* observer);
        void removeObserver(IButtonEventObserver* observer);

    private:
        std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)> buttonStates;
        std::vector<IButtonEventObserver*> observers;
        void notifyObservers(Button button, ButtonStateEnum state, uint32_t duration);
    };
} // namespace YFComms