#include "BoardConfig.hpp"

namespace YFComms {
    void loadBoardConfig500Classic(
        std::vector<BoardConfig::LEDConfig>& ledConfigs,
        std::vector<BoardConfig::ButtonConfig>& buttonConfigs) {
        ledConfigs = {
            {0, BoardConfig::CommunicationType::GPIO, 1}, // Beispiel für LIFTED LED
            {1, BoardConfig::CommunicationType::UART, 0}, // Beispiel für SIGNAL LED
            {2, BoardConfig::CommunicationType::UART, 0}, // BATTERY_LOW
            {3, BoardConfig::CommunicationType::UART, 0}, // CHARGING
            // Weitere LEDs...
        };

        buttonConfigs = {
            {0, BoardConfig::CommunicationType::GPIO, 5}, // Play Button
            {1, BoardConfig::CommunicationType::UART, 0}  // Home Button
            // Weitere Buttons...
        };
    }

} // namespace YFComms