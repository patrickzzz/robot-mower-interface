#pragma once

#include <memory>
#include "LEDState.hpp"
#include "ButtonState.hpp"
#include "YFCoverUIControllerUART.hpp"
#include "LEDControllerGPIO.hpp"
#include "ButtonControllerGPIO.hpp"

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
        std::unique_ptr<BoardConfig> boardConfig;

        // LED and Button states
        LEDState ledState;
        ButtonState buttonState;

        // YFCoverUIControllerUART
        std::unique_ptr<YFCoverUIControllerUART> yfCoverUIControllerUART;

        // LEDControllerGPIO
        std::unique_ptr<LEDControllerGPIO> ledControllerGPIO;

        // ButtonControllerGPIO
        std::unique_ptr<ButtonControllerGPIO> buttonControllerGPIO;

        // Private helpers
        void setupCommunicationHandler(); // Configures the CommunicationHandler based on BoardConfig
    };

} // namespace YFComms