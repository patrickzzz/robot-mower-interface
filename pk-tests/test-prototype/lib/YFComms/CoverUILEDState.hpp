#ifndef COVER_UI_LED_STATE_HPP
#define COVER_UI_LED_STATE_HPP

#include <vector>
#include <cstdint>

class CoverUILEDState {
public:
    enum class LEDState : uint8_t {
        OFF = 0x00,
        ON = 0x10,
        FLASH_SLOW = 0x20,
        FLASH_FAST = 0x22
    };

    static constexpr uint8_t NUM_LEDS = 19;

    CoverUILEDState();

    void setLEDState(uint8_t led, LEDState state);
    std::vector<uint8_t> getStatusMessage() const;

private:
    const std::vector<uint8_t> defaultStatusMessage;
    std::vector<LEDState> ledStates;
};

#endif // COVER_UI_LED_STATE_HPP