#pragma once

#include <memory>
#include "BoardConfig.hpp"
#include "LEDState.hpp"
#include "ButtonState.hpp"
#include "CommunicationHandler.hpp"

namespace YFComms {

    class CoverUIController {
    public:
        // Constructor
        CoverUIController(const std::string& modelName);

        // Initialize the controller (configure communication, LEDs, buttons)
        void initialize();

        // Update button states (reads from hardware)
        void updateButtonStates();

        // Update LEDs based on LEDState
        void updateLEDStates();

        // Access LED state
        LEDState& getLEDState();

        // Access Button state
        const ButtonState& getButtonState() const;

    private:
        // Board configuration
        std::unique_ptr<BoardConfig> boardConfig;

        // LED and Button states
        LEDState ledState;
        ButtonState buttonState;

        // Communication handler
        std::unique_ptr<CommunicationHandler> commHandler;

        // Private helpers
        void setupCommunicationHandler(); // Configures the CommunicationHandler based on BoardConfig
    };

} // namespace YFComms