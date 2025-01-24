#pragma once

#include "driver/gpio.h"
#include "esp_io_expander.h"

void SetupPins();

// ----- UARTs -----
// YF-Rev6-A Plug
constexpr gpio_num_t pinUartR6ARx = GPIO_NUM_18;
constexpr gpio_num_t pinUartR6ATx = GPIO_NUM_17;
// YF-Rev6-B Plug
constexpr gpio_num_t pinUartR6BRx = GPIO_NUM_38;
constexpr gpio_num_t pinUartR6BTx = GPIO_NUM_37;
// OM-UI-Board Plug
constexpr gpio_num_t pinUartOMRx = GPIO_NUM_44;
constexpr gpio_num_t pinUartOMTx = GPIO_NUM_43;

// ----- Port Expander (TCA9555). See usage example in bhs_sensors.hpp -----
constexpr uint8_t portExpAddr = 0x20;
constexpr gpio_num_t portExpINT = GPIO_NUM_34;  // Low-Active
constexpr gpio_num_t portExpSCL = GPIO_NUM_36;
constexpr gpio_num_t portExpSDA = GPIO_NUM_35;

// ----- HW Specific Pins -----
#ifdef HW_0_1_X

// ----- OnBoard LEDs -----
// Attention: PCB wires on v0.1 are routed on a "not recommended GPIO" which turned out are "not allowed" pins!
// const gpio_num_t pinLedGreen = GPIO_NUM_31;
// const gpio_num_t pinLedRed = GPIO_NUM_32;
// Manually fixed via wires
constexpr gpio_num_t pinLedGreen = GPIO_NUM_47;
constexpr gpio_num_t pinLedRed = GPIO_NUM_33;

constexpr gpio_num_t onBoardLeds[] = {pinLedGreen, pinLedRed};

// ----- CoverUI LEDs -----
constexpr gpio_num_t pinLedLifted = GPIO_NUM_1;
constexpr gpio_num_t pinLedConnect = GPIO_NUM_2;  // WLAN Symbol
constexpr gpio_num_t pinLedBattery = GPIO_NUM_3;  // Reverse meaning, with a no- or empty- battery symbol
constexpr gpio_num_t pinLedCharge = GPIO_NUM_4;   // Mostly with a blue LED

// All (LED) CoverUI's have 4 "hour" LEDs, but with different values, dependent on the model
constexpr gpio_num_t pinLed14h = GPIO_NUM_10;  // LED 2|4 hr
constexpr gpio_num_t pinLed24h = GPIO_NUM_11;  // LED 4|6 hr
constexpr gpio_num_t pinLed34h = GPIO_NUM_12;  // LED 6|8 hr
constexpr gpio_num_t pinLed44h = GPIO_NUM_13;  // LED 8|10 hr

// Attention: HW v0.1 used these "not recommended" GPIOs, which turned out are "not allowed" pins! Not manually fixed in HW v0.1
// constexpr gpio_num_t pinLedS1 = GPIO_NUM_27;
// constexpr gpio_num_t pinLedS2 = GPIO_NUM_28;
// constexpr gpio_num_t pinLedLock = GPIO_NUM_29;
// constexpr gpio_num_t pinLedBacklight = GPIO_NUM_30;  // LCD-Backlight LED

// FIXME: We should not init the LEDs which aren't avail (on "not allowed" GPIOs)
constexpr gpio_num_t coverUILeds[] = {
    pinLed14h, pinLed24h, pinLed34h, pinLed44h,
    // pinLedS1, pinLedS2, pinLedLock,
    // pinLedBacklight,
    pinLedLifted, pinLedConnect, pinLedBattery, pinLedCharge};

// Nearly all buttons are connected to the port expander. See usage in bhs_sensors.hpp
constexpr esp_io_expander_pin_num_t expPinStop1 = IO_EXPANDER_PIN_NUM_0;
constexpr esp_io_expander_pin_num_t expPinStop2 = IO_EXPANDER_PIN_NUM_1;
constexpr esp_io_expander_pin_num_t expPinLift = IO_EXPANDER_PIN_NUM_2;
constexpr esp_io_expander_pin_num_t expPinLiftX = IO_EXPANDER_PIN_NUM_3;
constexpr esp_io_expander_pin_num_t expPinBumpL = IO_EXPANDER_PIN_NUM_4;
constexpr esp_io_expander_pin_num_t expPinBumpR = IO_EXPANDER_PIN_NUM_5;
constexpr esp_io_expander_pin_num_t expPinBtnHome = IO_EXPANDER_PIN_NUM_6;
constexpr esp_io_expander_pin_num_t expPinShellStop1 = IO_EXPANDER_PIN_NUM_7;
constexpr esp_io_expander_pin_num_t expPinBtnLock = IO_EXPANDER_PIN_NUM_8;
constexpr esp_io_expander_pin_num_t expPinBtnS1 = IO_EXPANDER_PIN_NUM_9;
constexpr esp_io_expander_pin_num_t expPinBtnS2 = IO_EXPANDER_PIN_NUM_10;
constexpr esp_io_expander_pin_num_t expPinBtnHr14 = IO_EXPANDER_PIN_NUM_11;
constexpr esp_io_expander_pin_num_t expPinBtnHr24 = IO_EXPANDER_PIN_NUM_12;
constexpr esp_io_expander_pin_num_t expPinBtnHr34 = IO_EXPANDER_PIN_NUM_13;
constexpr esp_io_expander_pin_num_t expPinBtnHr44 = IO_EXPANDER_PIN_NUM_14;
constexpr esp_io_expander_pin_num_t expPinShellStop2 = IO_EXPANDER_PIN_NUM_15;

constexpr uint32_t portExpBtnPinMask = expPinStop1 | expPinStop2 | expPinLift | expPinLiftX | expPinBumpL | expPinBumpR | expPinBtnHome |
                                       expPinShellStop1 | expPinBtnLock | expPinBtnS1 | expPinBtnS2 |
                                       expPinBtnHr14 | expPinBtnHr24 | expPinBtnHr34 | expPinBtnHr44 |
                                       expPinShellStop2;

// Play button is connected directly to the ESP because it's also used as ESP32- boot-mode switch
constexpr gpio_num_t pinButtonPlay = GPIO_NUM_0;
// But got merged/overlayed to a unused port expander port for simpler app handling
constexpr esp_io_expander_pin_num_t mergedExpPinBtnPlay = IO_EXPANDER_PIN_NUM_16;

#elif HW_0_2_X

// ----- OnBoard LEDs -----
constexpr gpio_num_t pinLedGreen = GPIO_NUM_46;
constexpr gpio_num_t pinLedRed = GPIO_NUM_21;

constexpr gpio_num_t onBoardLeds[] = {pinLedGreen, pinLedRed};

// ----- CoverUI LEDs -----
constexpr gpio_num_t pinLedLifted = GPIO_NUM_1;
constexpr gpio_num_t pinLedConnect = GPIO_NUM_4;  // WLAN Symbol
constexpr gpio_num_t pinLedBattery = GPIO_NUM_2;  // Reverse meaning, with a no- or empty- battery symbol
constexpr gpio_num_t pinLedCharge = GPIO_NUM_6;   // Mostly with a blue LED

// All (LED) CoverUI's have 4 "hour" LEDs, but with different values, dependent on the model
constexpr gpio_num_t pinLed14h = GPIO_NUM_7;  // LED 2|4 hr
constexpr gpio_num_t pinLed24h = GPIO_NUM_8;  // LED 4|6 hr
constexpr gpio_num_t pinLed34h = GPIO_NUM_9;  // LED 6|8 hr
constexpr gpio_num_t pinLed44h = GPIO_NUM_5;  // LED 8|10 hr

constexpr gpio_num_t pinLedS1 = GPIO_NUM_11;
constexpr gpio_num_t pinLedS2 = GPIO_NUM_12;
constexpr gpio_num_t pinLedLock = GPIO_NUM_10;
constexpr gpio_num_t pinLedBacklight = GPIO_NUM_13;  // LCD-Backlight LED

constexpr gpio_num_t coverUILeds[] = {
    pinLed14h, pinLed24h, pinLed34h, pinLed44h,
    pinLedS1, pinLedS2, pinLedLock,
    pinLedBacklight,
    pinLedLifted, pinLedConnect, pinLedBattery, pinLedCharge};

// Nearly all buttons are connected to the port expander. See usage in bhs_sensors.hpp
constexpr esp_io_expander_pin_num_t expPinStop2 = IO_EXPANDER_PIN_NUM_0;
constexpr esp_io_expander_pin_num_t expPinLiftX = IO_EXPANDER_PIN_NUM_1;
constexpr esp_io_expander_pin_num_t expPinBumpL = IO_EXPANDER_PIN_NUM_2;
constexpr esp_io_expander_pin_num_t expPinStop1 = IO_EXPANDER_PIN_NUM_3;
constexpr esp_io_expander_pin_num_t expPinBumpR = IO_EXPANDER_PIN_NUM_4;
constexpr esp_io_expander_pin_num_t expPinLift = IO_EXPANDER_PIN_NUM_5;
constexpr esp_io_expander_pin_num_t expPinShellStop1 = IO_EXPANDER_PIN_NUM_6;
constexpr esp_io_expander_pin_num_t expPinShellStop2 = IO_EXPANDER_PIN_NUM_7;
constexpr esp_io_expander_pin_num_t expPinBtnHr14 = IO_EXPANDER_PIN_NUM_8;
constexpr esp_io_expander_pin_num_t expPinBtnHr24 = IO_EXPANDER_PIN_NUM_9;
constexpr esp_io_expander_pin_num_t expPinBtnHr34 = IO_EXPANDER_PIN_NUM_10;
constexpr esp_io_expander_pin_num_t expPinBtnHr44 = IO_EXPANDER_PIN_NUM_11;
constexpr esp_io_expander_pin_num_t expPinBtnLock = IO_EXPANDER_PIN_NUM_12;
constexpr esp_io_expander_pin_num_t expPinBtnS2 = IO_EXPANDER_PIN_NUM_13;
constexpr esp_io_expander_pin_num_t expPinBtnS1 = IO_EXPANDER_PIN_NUM_14;

constexpr uint32_t portExpBtnPinMask = expPinStop1 | expPinStop2 | expPinLift | expPinLiftX | expPinBumpL | expPinBumpR |
                                       expPinShellStop1 | expPinBtnLock | expPinBtnS1 | expPinBtnS2 |
                                       expPinBtnHr14 | expPinBtnHr24 | expPinBtnHr34 | expPinBtnHr44 |
                                       expPinShellStop2;

// Play & Home buttons are connected directly to the ESP because they've extended functionality also used as ESP32- boot-mode switch
constexpr gpio_num_t pinButtonPlay = GPIO_NUM_0;  // Also used for ESP "boot mode"
constexpr gpio_num_t pinButtonHome = GPIO_NUM_3;  // Also used for ESP "external JTAG"
// But got merged/overlayed to a unused port expander port for simpler app handling
constexpr esp_io_expander_pin_num_t mergedExpPinBtnPlay = IO_EXPANDER_PIN_NUM_16;
constexpr esp_io_expander_pin_num_t mergedExpPinBtnHome = IO_EXPANDER_PIN_NUM_17;

#else
#error No hardware version HW_x_y_z defined
#endif