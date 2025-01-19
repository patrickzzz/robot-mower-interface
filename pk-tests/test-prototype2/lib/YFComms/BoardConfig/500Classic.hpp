#include "BoardConfig.hpp"
#include "driver/gpio.h"
#include "esp_io_expander.h"
#include "LEDState.hpp"

namespace YFComms {
    void loadBoardConfig500Classic(
        std::vector<BoardConfig::LEDConfig>& ledConfigs,
        std::vector<BoardConfig::ButtonConfig>& buttonConfigs) {
        ledConfigs = {
            {static_cast<uint8_t>(LED::BATTERY_LOW), BoardConfig::CommunicationType::UART, 0xFF, 2},
            {static_cast<uint8_t>(LED::CHARGING), BoardConfig::CommunicationType::UART, 0xFF, 3}
            // Weitere LEDs...
        };

        buttonConfigs = {
            {0, BoardConfig::CommunicationType::GPIO, 5, 0xFFFFFFFF}, // Play Button
            // Weitere Buttons...
        };
    }

} // namespace YFComms