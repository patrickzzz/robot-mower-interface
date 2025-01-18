#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace YFComms {

    class BoardConfig {
    public:
        enum class CommunicationType {
            GPIO,        // item is accessed via GPIO
            GPIO_EXP,    // item is accessed via GPIO expander
            UART         // item is accessed via UART
        };

        struct LEDConfig {
            uint8_t ledIndex;
            CommunicationType commType;
            uint8_t gpioPin;
        };

        struct ButtonConfig {
            uint8_t buttonIndex;
            CommunicationType commType;
            uint8_t gpioPin;
            uint8_t expanderPin;
        };

        BoardConfig(std::string modelName);

        const std::vector<LEDConfig>& getLEDConfigs() const;
        const std::vector<ButtonConfig>& getButtonConfigs() const;

        std::string getModelName() const;

    private:
        std::string modelName;
        std::vector<LEDConfig> ledConfigs;
        std::vector<ButtonConfig> buttonConfigs;

        void setupConfigs(); // Initialize the LED and Button configurations
        void loadConfig(); // Load the configuration from a file
    };

} // YFComms