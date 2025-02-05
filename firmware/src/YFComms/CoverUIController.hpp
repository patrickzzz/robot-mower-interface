#pragma once

#include <memory>
#include "State/LEDState.hpp"
#include "State/ButtonState.hpp"
#include "UART/CoverUIControllerUART.hpp"
#include "GPIO/LEDControllerGPIO.hpp"
#include "GPIO/ButtonAndSensorAdapterGPIO.hpp"
#include "BoardConfig/AbstractBoardConfig.hpp"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "UART/VirtualMainboardUART.hpp"
#include "UART/HardwareMainboardUART.hpp"

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

        // Access LED state
        LEDState& getLEDState();

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

        // LED and Button states
        LEDState ledState;
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
    };

} // namespace YFComms