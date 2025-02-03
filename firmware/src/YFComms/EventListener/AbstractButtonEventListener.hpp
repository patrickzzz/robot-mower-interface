#pragma once

#include "../State/ButtonState.hpp"

namespace YFComms {
    // Forward declaration
    enum class Button : uint8_t;
    enum class ButtonStateEnum : uint8_t;

    class AbstractButtonEventListener {
    public:
        virtual ~AbstractButtonEventListener() = default;
        virtual void onButtonEvent(Button button, ButtonStateEnum state, uint32_t duration) = 0;

    protected:
        std::string getButtonName(Button button);
    };
} // namespace YFComms