#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

namespace YFComms {
    // All possible buttons
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

    // All possible button states
    enum class ButtonStateEnum : uint8_t {
        RELEASED = 0x00,
        PRESSED = 0x01
    };

    class ButtonState {
    public:
        ButtonState(); // Constructor

        // Set state of a Button
        ButtonState& setState(Button button, ButtonStateEnum state);

        // Get state of a Button
        ButtonStateEnum getState(Button button) const;

        // Get all Button states
        const std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)>& getStates() const;

        // Check if the state of any Button has been updated
        bool getIsUpdated() const { return isUpdated; }
        void setIsUpdated(bool updated) { isUpdated = updated; }

    private:
        bool isUpdated = false;

        // Array to store the state of all Buttons
        std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)> buttonStates;
    };
} // namespace YFComms