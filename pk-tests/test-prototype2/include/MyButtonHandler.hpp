#pragma once

#include "ButtonState.hpp"
#include "IButtonEventObserver.hpp"
#include <esp_log.h>

class MyButtonHandler : public YFComms::IButtonEventObserver {
public:
    void onButtonEvent(YFComms::Button button, YFComms::ButtonStateEnum state, uint32_t duration) override {
        if(state == YFComms::ButtonStateEnum::PRESSED) {
            ESP_LOGI("MyButtonHandler", "Button %u is now pressed", static_cast<int>(button));
        }else{
            ESP_LOGI("MyButtonHandler", "Button %u released after %u ms", static_cast<int>(button), static_cast<int>(duration));
        }
    }
};