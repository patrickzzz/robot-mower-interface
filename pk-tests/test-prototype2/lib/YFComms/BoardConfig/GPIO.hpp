#include "BoardConfig.hpp"

namespace YFComms {
    void loadBoardConfigGPIO(
        std::vector<BoardConfig::LEDConfig>& ledConfigs,
        std::vector<BoardConfig::ButtonConfig>& buttonConfigs) {
        ledConfigs = {
            {0, BoardConfig::CommunicationType::GPIO, GPIO_NUM_1},    // Lifted
            {1, BoardConfig::CommunicationType::GPIO, GPIO_NUM_2},    // Connect (WLAN Symbol)
            {2, BoardConfig::CommunicationType::GPIO, GPIO_NUM_3},    // Battery
            {3, BoardConfig::CommunicationType::GPIO, GPIO_NUM_4},    // Charge
            {11, BoardConfig::CommunicationType::GPIO, GPIO_NUM_10},  // 4 hr
            {12, BoardConfig::CommunicationType::GPIO, GPIO_NUM_11},  // 6 hr
            {13, BoardConfig::CommunicationType::GPIO, GPIO_NUM_12},  // 8 hr
            {14, BoardConfig::CommunicationType::GPIO, GPIO_NUM_13}   // 10 hr
            // not working leds
            // {4, BoardConfig::CommunicationType::GPIO, GPIO_NUM_27},  // S1
            // {5, BoardConfig::CommunicationType::GPIO, GPIO_NUM_28},  // S2
            // {6, BoardConfig::CommunicationType::GPIO, GPIO_NUM_29},  // Lock
            // {9, BoardConfig::CommunicationType::GPIO, GPIO_NUM_30}   // Backlight
        };

        buttonConfigs = {
            {0, BoardConfig::CommunicationType::GPIO, GPIO_NUM_0}, // Play Button
            {1, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_6},  // Home Button
            {10, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_8}, // Lock
            {11, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_9}, // S1
            {12, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_10}, // S2
            {13, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_11}, // 4 hr
            {14, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_12}, // 6 hr
            {15, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_13}, // 8 hr
            {16, BoardConfig::CommunicationType::GPIO_EXP, 0xFF, IO_EXPANDER_PIN_NUM_14}, // 10 hr
        };
    }
} // namespace YFComms