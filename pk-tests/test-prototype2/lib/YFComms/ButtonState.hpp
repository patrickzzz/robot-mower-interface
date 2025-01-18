#pragma once

#include <array>
#include <cstdint>
#include <cstdio>

// this is for now just a demo/wip implementation

namespace YFComms {
    // all possible Buttons
    enum class Button : uint8_t {
        PLAY,         // 0
        HOME,         // 1
        STOP,         // 2
        LIFT,         // 3
        S1,           // 4
        S2,           // 5
        MAX           // 6
    };

    // all possible Button states
    enum class ButtonStateEnum : uint8_t {
        RELEASED = 0x00,
        PRESSED = 0x01
    };

    class ButtonState {
    public:
        ButtonState(); // Constructor

        // Set state of a Button
        void setButtonState(Button button, ButtonStateEnum state);

        // Get state of a Button
        ButtonStateEnum getButtonState(Button button) const;

        // Get all Button states
        const std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)>& getButtonStates() const;

        // Set all Buttons to a specific state
        void setAllButtonStates(ButtonStateEnum state);

        // Debugging: Print the state of all Buttons to the console
        void printButtonStates() const;

    private:
        // Array to store the state of all Buttons
        std::array<ButtonStateEnum, static_cast<size_t>(Button::MAX)> buttonStates;

        // Helper function: Convert Button state to readable text
        static const char* stateToString(ButtonStateEnum state);
    };
} // namespace YFComms