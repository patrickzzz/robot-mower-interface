#pragma once

#include <array>
#include <cstdint>
#include <cstdio>

namespace YFComms {
    // all possible LEDs
    enum class LED : uint8_t {
        LIFTED,        // 0
        SIGNAL,        // 1
        BATTERY_LOW,   // 2
        CHARGING,      // 3
        S1,            // 4
        S2,            // 5
        LOCK,          // 6
        WIFI,          // 7
        GPS,           // 8
        BACKGROUND,    // 9
        HOURS_TWO,     // 10
        HOURS_FOUR,    // 11
        HOURS_SIX,    // 12
        HOURS_EIGHT,   // 13
        HOURS_TEN,     // 14
        DAY_MON,       // 15
        DAY_TUE,       // 16
        DAY_WED,       // 17
        DAY_THR,       // 18
        DAY_FRI,       // 19
        DAY_SAT,       // 20
        DAY_SUN,       // 21
        MAX            // 22
    };

    // all possible led states
    enum class LEDStateEnum : uint8_t {
        OFF = 0x00,
        ON = 0x10,
        FLASH_SLOW = 0x20,
        FLASH_FAST = 0x22
    };

    class LEDState {
        public:
        LEDState(); // Constructor

        // Set state of a LED
        LEDState& setState(LED led, LEDStateEnum state);

        // Get state of a LED
        LEDStateEnum getState(LED led) const;

        // Get all LED states
        const std::array<LEDStateEnum, static_cast<size_t>(LED::MAX)>& getStates() const;

        // Check if the state of any LED has been updated
        bool getIsUpdated() const { return isUpdated; }
        void setIsUpdated(bool updated) { isUpdated = updated; }

    private:
        // isUpdated flag, as long as LEDs are only either set on Serial or GPIO, one flag is enough
        bool isUpdated = false;

        // Array to store the state of all LEDs
        std::array<LEDStateEnum, static_cast<size_t>(LED::MAX)> ledStates;
    };
} // namespace YFComms