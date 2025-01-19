#include "BoardConfig.hpp"
#include "BoardConfig/GPIO.hpp"
#include "BoardConfig/500Classic.hpp"

namespace YFComms {
    BoardConfig::BoardConfig(std::string modelName)
        : modelName(std::move(modelName)) {
        setupConfigs();
    }

    const std::vector<BoardConfig::LEDConfig>& BoardConfig::getLEDConfigs() const {
        return ledConfigs;
    }

    const std::vector<BoardConfig::ButtonConfig>& BoardConfig::getButtonConfigs() const {
        return buttonConfigs;
    }

    std::string BoardConfig::getModelName() const {
        return modelName;
    }

    void BoardConfig::setupConfigs() {
        if (modelName == "GPIO") {
            loadBoardConfigGPIO(ledConfigs, buttonConfigs);
        } else if (modelName == "500Classic") {
            loadBoardConfig500Classic(ledConfigs, buttonConfigs);
        }
    }

} // namespace YFComms