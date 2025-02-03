#pragma once

#include "BoardConfigGPIO.hpp"
#include "BoardConfig500Classic.hpp"

namespace YFComms {
    std::unique_ptr<AbstractBoardConfig> BoardConfigFactory(const std::string& modelName) {
        if (modelName == "GPIO") {
            return std::make_unique<BoardConfigGPIO>();
        } else if (modelName == "500Classic") {
            return std::make_unique<BoardConfig500Classic>();
        } else {
            return nullptr;
        }
    }
} // namespace YFComms