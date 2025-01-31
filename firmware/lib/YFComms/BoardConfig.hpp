#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <array>

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
            uint8_t uartMessagePos;
        };

        virtual ~BoardConfig() = default;

        virtual const LEDConfig* getLEDConfigs(size_t& count) const = 0;
        virtual const ButtonConfig* getButtonConfigs(size_t& count) const = 0;

        std::string getModelName() const { return modelName; }
        virtual bool hasSerialCommunication() const = 0;
        virtual const uint8_t* getDefaultLEDMessage(size_t& length) const = 0;

    protected:
        explicit BoardConfig(std::string modelName) : modelName(std::move(modelName)) {}
        std::string modelName;
    };

} // YFComms