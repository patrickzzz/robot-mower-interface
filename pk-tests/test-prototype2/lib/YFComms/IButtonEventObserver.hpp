#pragma once

#include "ButtonState.hpp"

namespace YFComms {
    // Forward declaration
    enum class Button : uint8_t;
    enum class ButtonStateEnum : uint8_t;

    class IButtonEventObserver {
    public:
        virtual ~IButtonEventObserver() = default;

        virtual void onButtonEvent(Button button, ButtonStateEnum state, uint32_t duration) = 0;
    };
} // namespace YFComms