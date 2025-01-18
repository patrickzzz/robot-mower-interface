#include "pins.h"

/**
 * @brief SetupPins will setup only generic pins with direct access,
 * but not those which are handled by a separate lib like I2C or bhs_sensors.
 */
void SetupPins() {
    uint64_t pin_bit_mask = 0;
    gpio_config_t gpio_conf;

    // OnBoard LEDs get set always to OUTPUT mode
    for (auto pin : onBoardLeds) {
        pin_bit_mask |= (1ULL << pin);
    }
    gpio_conf = {
        .pin_bit_mask = pin_bit_mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&gpio_conf);

    // CoverUI LEDs need to be set in INPUT or OUTPUT mode, dependent on Mainboard, CoverUI or Revision!
    // @patrickzzz For testing purposed I only used OUTPUT, but you need something configurable, also for INPUT here ;-)
    pin_bit_mask = 0;
    for (auto pin : coverUILeds) {
        pin_bit_mask |= (1ULL << pin);
    }
    gpio_conf = {
        .pin_bit_mask = pin_bit_mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&gpio_conf);
}