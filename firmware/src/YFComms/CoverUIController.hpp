#pragma once

#include <memory>
#include "State/ButtonState.hpp"
#include "UART/CoverUIControllerUART.hpp"
#include "GPIO/LEDControllerGPIO.hpp"
#include "GPIO/ButtonAndSensorAdapterGPIO.hpp"
#include "BoardConfig/AbstractBoardConfig.hpp"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "UART/VirtualMainboardUART.hpp"
#include "UART/HardwareMainboardUART.hpp"
#include "LED.hpp"

namespace YFComms {

    struct UARTConfig {
        uart_port_t uartPort;
        gpio_num_t rxPin;
        gpio_num_t txPin;
    };

    // ENUM for Mainboard-Types
    enum class MainboardType {
        UNDEFINED,
        YF_HARDWARE,
        YF_VIRTUAL
    };

    class CoverUIController {
    public:
        CoverUIController(const std::string& modelName);

        // change model
        void changeModel(const std::string& modelName);

        // Initialize the controller (configure communication, LEDs, buttons)
        void initialize();

        /**
         * @brief Get the current LED mode
         *
         * @param name
         * @return led::Modes
         */
        led::Modes getLEDMode(led::Names name) const {
            const auto& it = boardConfig->leds->find(name);  // Find the named LED within our leds map
            if (it != boardConfig->leds->end()) {            // Found
                auto led = it->second;
                return led.getMode();
            }
            return led::Modes::OFF;
        }

        /**
         * @brief Set the mode of the LED and track if the mode changed
         *
         * @param name of the LED
         * @param state like ON, OFF, FLASH_...
         */
        void setLEDMode(const led::Names name, const led::Modes mode) {
            auto it = boardConfig->leds->find(name);  // Find the named LED within our leds map
            if (it != boardConfig->leds->end()) {     // Found
                auto led = it->second;
                led_mode_changed_ |= led.setMode(mode);  // Logic OR result with LED-Changed track flag
            }  // else: LED with this name does'nt exists
        }

        /**
         * @brief Set the all LEDs to the given mode
         * 
         * @param mode 
         */
        void setAllLEDsMode(const led::Modes mode) {
            for (auto& [name, led] : *boardConfig->leds) {
                setLEDMode(name, mode);
            }
        }

        /**
         * @brief Has a LED mode changed (since last commit?)?
         *
         * @return true if the LED mode changed
         * @return false if no LED mode changed
         */
        bool hasLEDModeChanged() { return led_mode_changed_; }

        void setLEDModeChanged(const bool changed) { led_mode_changed_ = changed; }

        // Access Button state
        ButtonState& getButtonState();

        void setMainboardType(MainboardType mainboardType) {
            this->mainboardType = mainboardType;
        }
        void setUARTConfigCoverUI(const UARTConfig& uartConfig) {
            uartConfigCoverUI = uartConfig;
        }
        void setUARTConfigHardwareMainboard(const UARTConfig& uartConfig) {
            uartConfigHardwareMainboard = uartConfig;
        }

    private:
        // Board configuration
        std::unique_ptr<AbstractBoardConfig> boardConfig;

        // Button states
        ButtonState buttonState;

        // Lets create a virtual mainboard
        std::unique_ptr<VirtualMainboardUART> virtualMainboardUART;

        // Lets create a HardwareMainboardUART
        std::unique_ptr<HardwareMainboardUART> hardwareMainboardUART;

        // CoverUIControllerUART
        std::unique_ptr<CoverUIControllerUART> coverUIControllerUART;

        // LEDControllerGPIO
        std::unique_ptr<LEDControllerGPIO> ledControllerGPIO;

        // ButtonAndSensorAdapterGPIO
        std::unique_ptr<ButtonAndSensorAdapterGPIO> buttonAndSensorAdapterGPIO;

        // mainboard type
        MainboardType mainboardType = MainboardType::UNDEFINED;

        // UART configuration for CoverUIControllerUART
        UARTConfig uartConfigCoverUI;

        // UART configuration for HardwareMainboardUART
        UARTConfig uartConfigHardwareMainboard;

        // Private helpers
        void setupCommunicationHandler(); // Configures the CommunicationHandler based on AbstractBoardConfig

        // Tracks if a LED state got changed
        bool led_mode_changed_ = false;
    };
} // namespace YFComms