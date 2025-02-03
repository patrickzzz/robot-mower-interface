#pragma once

#include <memory>
#include "State/LEDState.hpp"
#include "State/ButtonState.hpp"
#include "UART/YFCoverUIControllerUART.hpp"
#include "GPIO/LEDControllerGPIO.hpp"
#include "GPIO/ButtonAndSensorAdapterGPIO.hpp"
#include "BoardConfig/AbstractBoardConfig.hpp"

namespace YFComms {

    class CoverUIController {
    public:
        // Constructor
        CoverUIController(const std::string& modelName);

        // change model
        void changeModel(const std::string& modelName);

        // Initialize the controller (configure communication, LEDs, buttons)
        void initialize();

        // Access LED state
        LEDState& getLEDState();

        // Access Button state
        ButtonState& getButtonState();

    private:
        // Board configuration
        std::unique_ptr<AbstractBoardConfig> boardConfig;

        // LED and Button states
        LEDState ledState;
        ButtonState buttonState;

        // YFCoverUIControllerUART
        std::unique_ptr<YFCoverUIControllerUART> yfCoverUIControllerUART;

        // LEDControllerGPIO
        std::unique_ptr<LEDControllerGPIO> ledControllerGPIO;

        // ButtonAndSensorAdapterGPIO
        std::unique_ptr<ButtonAndSensorAdapterGPIO> buttonAndSensorAdapterGPIO;

        // Private helpers
        void setupCommunicationHandler(); // Configures the CommunicationHandler based on AbstractBoardConfig
    };

} // namespace YFComms