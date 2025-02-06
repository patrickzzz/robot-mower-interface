#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <array>
#include "etl/flat_map.h"

namespace YFComms {
    // Forward declare LED dependencies to get away of looped include hell.
    // Explanation: We like to #include "../LED.hpp" but LED.hpp also need this header file.
    //    Forward declaration of the required definitions in LED.hpp does not work, because
    //    the required definition is within AbstractBoardConfig Class and those are not forward declarable.
    //    Thats why we need to forward declare it here (as long as the structure does'nt get cleaned up).
    namespace led {
        enum class Names;
        class LED;
    }
    
    class AbstractBoardConfig {
    public:
        enum class CommunicationType {
            GPIO,        // item is accessed via GPIO
            GPIO_EXP,    // item is accessed via GPIO expander
            UART         // item is accessed via UART
        };

        struct ButtonConfig {
            uint8_t buttonIndex;
            CommunicationType commType;
            uint8_t gpioPin;
            uint32_t expanderPin;
            uint8_t uartMessagePos;
        };

        etl::iflat_map<YFComms::led::Names, YFComms::led::LED>* leds; // Pointer to our LED map which get assigned in derived class

        virtual ~AbstractBoardConfig() = default;

        virtual const ButtonConfig* getButtonConfigs(size_t& count) const = 0;

        std::string getModelName() const { return modelName; }
        virtual bool hasSerialCommunication() const = 0;
        virtual const uint8_t* getDefaultLEDMessage(size_t& length) const = 0;

    protected:
        explicit AbstractBoardConfig(std::string modelName) : modelName(std::move(modelName)) {}
        std::string modelName;
    };

} // YFComms