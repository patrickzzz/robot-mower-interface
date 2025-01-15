#include "CoverUILEDState.hpp"

CoverUILEDState::CoverUILEDState()
    : defaultStatusMessage{
        0x55, 0xAA, 0x15, 0x50, 0x8E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00},
    ledStates(NUM_LEDS, LEDState::OFF)
{
}

void CoverUILEDState::setLEDState(uint8_t led, LEDState state) {
    if (led < NUM_LEDS) {
        ledStates[led] = state;
    }
}

std::vector<uint8_t> CoverUILEDState::getStatusMessage() const {
    std::vector<uint8_t> statusMessage = defaultStatusMessage;

    for (size_t i = 0; i < ledStates.size(); ++i) {
        statusMessage[5 + i] = static_cast<uint8_t>(ledStates[i]);
    }

    return statusMessage;
}