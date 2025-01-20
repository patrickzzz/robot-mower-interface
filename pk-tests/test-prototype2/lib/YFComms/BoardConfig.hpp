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
            uint8_t uartMessagePos;
        };

        struct ButtonConfig {
            uint8_t buttonIndex;
            CommunicationType commType;
            uint8_t gpioPin;
            uint32_t expanderPin;
        };

        virtual ~BoardConfig() = default;

        virtual const std::vector<LEDConfig>& getLEDConfigs() const = 0;
        virtual const std::vector<ButtonConfig>& getButtonConfigs() const = 0;

        std::string getModelName() const { return modelName; }

    protected:
        explicit BoardConfig(std::string modelName) : modelName(std::move(modelName)) {}
        std::string modelName;
    };

} // YFComms