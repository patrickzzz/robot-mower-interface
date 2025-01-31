#pragma once

#include "ButtonState.hpp"
#include "IButtonEventObserver.hpp"
#include <esp_log.h>

class MyButtonHandler : public YFComms::IButtonEventObserver {
public:
    void onButtonEvent(YFComms::Button button, YFComms::ButtonStateEnum state, uint32_t duration) override {
        std::string buttonName = getButtonName(button);

        if (state == YFComms::ButtonStateEnum::HIGH) {
            ESP_LOGI("MyButtonHandler", "Button %s is now high after %u ms", buttonName.c_str(), static_cast<unsigned int>(duration));
        } else {
            ESP_LOGI("MyButtonHandler", "Button %s is now low after %u ms", buttonName.c_str(), static_cast<unsigned int>(duration));
        }
    }


private:
std::string getButtonName(YFComms::Button button) {
    switch (button) {
        case YFComms::Button::PLAY: return "PLAY";
        case YFComms::Button::HOME: return "HOME";
        case YFComms::Button::LOCK: return "LOCK";
        case YFComms::Button::OK: return "OK";
        case YFComms::Button::S1: return "S1";
        case YFComms::Button::S2: return "S2";
        case YFComms::Button::CLOCK: return "CLOCK";
        case YFComms::Button::DAYS_MON: return "DAYS_MON";
        case YFComms::Button::DAYS_TUE: return "DAYS_TUE";
        case YFComms::Button::DAYS_WED: return "DAYS_WED";
        case YFComms::Button::DAYS_THR: return "DAYS_THR";
        case YFComms::Button::DAYS_FRI: return "DAYS_FRI";
        case YFComms::Button::DAYS_SAT: return "DAYS_SAT";
        case YFComms::Button::DAYS_SUN: return "DAYS_SUN";

        case YFComms::Button::STOP1: return "STOP1";
        case YFComms::Button::STOP2: return "STOP2";
        case YFComms::Button::LIFT: return "LIFT";
        case YFComms::Button::LIFTX: return "LIFTX";
        case YFComms::Button::BUMPL: return "BUMPL";
        case YFComms::Button::BUMPR: return "BUMPR";

        case YFComms::Button::SHELL_STOP1: return "SHELL_STOP1";
        case YFComms::Button::SHELL_STOP2: return "SHELL_STOP2";

        default: return "UNKNOWN";
    }
};
};