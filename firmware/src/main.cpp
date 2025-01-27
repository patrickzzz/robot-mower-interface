/*
 * AH20241203 infos for @patrickzzz:
 * - Quick started this test-prototype code as described in official docs
 *   https://docs.platformio.org/en/latest/tutorials/espressif32/espidf_debugging_unit_testing_analysis.html#tutorial-espressif32-espidf-debugging-unit-testing-analysis
 *   but stopped at point 6 (Serial Monitor)
 * - Partitioned with 2*OTA "Safe update mode" see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html
 * - Followed Google C++ Sytle Guide see https://google.github.io/styleguide/cppguide.html#Naming except for "k" prefix for constants which I don't liked to follow ;-)
 * - Must read/best ESP-IDF reference guide https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32s3 Awesome ESP lib, much better than Arduino!!
 * - Our currently used ESP32-S3FH4R2 also has fast 2MB (Quad SPI) PSRAM!
 *   Whenever you need some more RAM for larger (>100? byte) buffer or similar, use the PSRAM specific malloc functions to use the PSRAM (instead of heap or stack)
 */

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <string.h>

#include "LedSequencer.hpp"
#include "config.h"
#include "pins.h"
#include "CoverUIController.hpp"
#include "MyButtonHandler.hpp"
// ToDo: remove the following 😪
#include "bhs_sensors.hpp"
#include "mainboards/mainboard_driver_om.hpp"

static const char* TAG = "Robot Mower Interface";

// OnBoard status LEDs
LedSequencer led_grn_seq(pinLedGreen);
LedSequencer led_red_seq(pinLedRed);

// We always have one (and only one) Mainboard Device/Interface
mainboards::MainboardInterface* mainboard;

SystemHealth sys_health = {};

#define LED_SEQ_FATAL_ERROR led_red_seq.blink({.on_ms = 100, .off_ms = 100, .fulfill = true})                           // Infinite quick red LED flash
#define LED_SEQ_COMMS_ERROR led_red_seq.blink({.on_ms = 100, .off_ms = 100, .limit_blink_cycles = 1, .fulfill = true})  // One quick red flash indicate a communication error

/**
 * @brief Task which handles all LED (output?)
 *
 */
void LedsTask(void* arg) {
    while (1) {
        led_grn_seq.loop();
        led_red_seq.loop();
        vTaskDelay(ledsTaskCycle / portTICK_PERIOD_MS);
    }
}

extern "C" void app_main(void) {
    vTaskDelay(2000 / portTICK_PERIOD_MS);  // Spiritless delay for monitor. Delay depends on host PCs performance. Increase if necessary.
    ESP_LOGI(TAG, "."); // Just to get a new line in the monitor
    ESP_LOGI(TAG, "Firmware Version %d.%02d", firmwareVersion / 100, firmwareVersion % 100);

    // Initialize NVS
    // @patrickzzz Possible to use NVS instead of SPIFFS for your WiFi/WebServer?
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "Erase NVS flash");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);  // Abort if NVS init fails

    // Setup generic pins, which get not initialized in a separate peripheral lib
    SetupPins();

    // Create LedsTask (currently only sequencer) with a very low priority
    // This should be made as early as possible to use LED blink codes for easier visual debugging
    TaskHandle_t leds_task_handle = NULL;
    xTaskCreate(LedsTask, "LEDs_Task", 1024, NULL, 2, &leds_task_handle);

    // LED test and blink sequence indicating "Minimal setup successful"
    led_grn_seq.blink({.limit_blink_cycles = 1, .fulfill = true});  // Default = 200ms ON, 200ms OFF
    led_red_seq.blink({.limit_blink_cycles = 1, .fulfill = true});  // Default = 200ms ON, 200ms OFF

    // @patrickzzz WiFi & WebServer init should start somewhere here.
    // Mainly because WiFi doesn't depend on any of the following peripherals.
    // But backend should check for sys_health if all peripherals are healthy and report in frontend if not!
    // Example how you could fail with a mandatory peripheral (i.e. WiFi or Webserver) error
    if (esp_err_t ret = ESP_OK != ESP_OK) {  // Replace first ESP_OK with your WiFi and/or Webserver init which return with a esp_err_t
        ESP_LOGE(TAG, "Example failed with error: %s", esp_err_to_name(ret));
        LED_SEQ_FATAL_ERROR;
        return;  // Other tasks will still run!
    }

    /*
    // Init button, hall & switch sensors lib
    if (esp_err_t ret = bhs_sensors::init(sys_health) != ESP_OK) {
        ESP_LOGE(TAG, "ButtonHallSwitch-Sensors lib failed with error: %s", esp_err_to_name(ret));
        led_red_seq.blink({.limit_blink_cycles = 3, .fulfill = true, .repeat = true});
        return;  // Other tasks will still run!
    }
    sys_health.port_expander = true;


    // Create BHS_Task with a very low priority for reading and debouncing all (port expander & GPIO) buttons
    TaskHandle_t bhs_sensors_task_handle = NULL;
    xTaskCreate(bhs_sensors::task, "BHSS_Task", 2048, NULL, 5, &bhs_sensors_task_handle);
    */

    // Example how the MainboardDriver could be used (TBD: when and where)
    // ATTENTION: Object will go into heap, but heap is limited to 4KB in ESP32 and there's no heap fragmentation protection nor heap overflow detection for most MCUs/frameworks
    mainboard = new mainboard_om::MainboardDriverOM(UART_NUM_2, pinUartOMRx, pinUartOMTx, nullptr);  // CB not implemented yet
    if (esp_err_t ret = mainboard->init() != ESP_OK) {
        ESP_LOGE(TAG, "MainboardDriverOM->init() failed with error: %s", esp_err_to_name(ret));
        led_red_seq.blink({.limit_blink_cycles = 5, .fulfill = true, .repeat = true});
        return;  // Other tasks will still run!
    }
    // This would destroy/release this mainboard instance (and will probably fragment the heap)
    // delete mainboard;

#ifdef DEV_PK
    // create coverUIController for 40 pin gpio
    YFComms::CoverUIController coverUIController("500Classic");
    coverUIController.initialize();

    // add Button/Hall Event Observer
    MyButtonHandler myButtonHandler;
    coverUIController.getButtonState().addObserver(&myButtonHandler);

    // enable leds
    coverUIController.getLEDState()
        .setState(YFComms::LED::LIFTED, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::SIGNAL, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::BATTERY_LOW, YFComms::LEDStateEnum::FLASH_SLOW);

        /*
        .setState(YFComms::LED::CHARGING, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::S1, YFComms::LEDStateEnum::FLASH_SLOW)
        .setState(YFComms::LED::S2, YFComms::LEDStateEnum::FLASH_SLOW)
        .setState(YFComms::LED::LOCK, YFComms::LEDStateEnum::FLASH_SLOW)
        .setState(YFComms::LED::HOURS_TWO, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::HOURS_FOUR, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::HOURS_SIX, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::HOURS_EIGHT, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::HOURS_TEN, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::DAY_MON, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::DAY_TUE, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::DAY_WED, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::DAY_THR, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::DAY_FRI, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::DAY_SAT, YFComms::LEDStateEnum::ON)
        .setState(YFComms::LED::DAY_SUN, YFComms::LEDStateEnum::ON);
*/
 //   vTaskDelay(4000 / portTICK_PERIOD_MS);  // delay(1000)
    // switch to 500c and update led states there as well..
 //   coverUIController.changeModel("500Classic");
#endif

    ESP_LOGI(TAG, "Start");
#ifdef DEV_PK
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // delay(1000)
    }
#endif

#ifdef DEV_AH
    while (1) {
        // app_main task "alive" flash
        led_grn_seq.blink({.on_ms = 20, .limit_blink_cycles = 1, .fulfill = true});
/*
        // How to get (debounced) button, hall or switch state:
        // Variant 1: Directly check one BHS-Sensor
        if (bhs_sensors::getState().shell_stop_1) ESP_LOGI(TAG, "Shell-Stop-1 closed");

        // Variant 2: Get State for all BHS-Sensors, and check their state afterwards, also for the possibility to test multiple states at once
        bhs_sensors::State bhs_state = bhs_sensors::getState();
        if (bhs_state.shell_stop_2 || bhs_state.btn_play) ESP_LOGI(TAG, "Shell-Stop-2 closed, or Play button pressed");
        if (bhs_state.btn_home) ESP_LOGI(TAG, "Home button pressed");
        if (bhs_state.btn_lock) ESP_LOGI(TAG, "Lock button pressed");
        if (bhs_state.btn_hr_14) ESP_LOGI(TAG, "14h button pressed");
        if (bhs_state.btn_hr_24) ESP_LOGI(TAG, "24h button pressed");
        if (bhs_state.btn_hr_34) ESP_LOGI(TAG, "34h button pressed");
        if (bhs_state.btn_hr_44) ESP_LOGI(TAG, "44h button pressed");

        // @patrickzzz If interested I could also add some kind of event functionality for every pressed button. I.e. like calling a predefined button-callback
 */
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // delay(1000)

        // Let's output some task/stack critical values to be monitored during development
        ESP_LOGI(TAG, "Task high water mark (free stack words) of: LEDs_Task %d",
                 //uxTaskGetStackHighWaterMark(leds_task_handle), uxTaskGetStackHighWaterMark(bhs_sensors_task_handle));
                 uxTaskGetStackHighWaterMark(leds_task_handle));
    }
#endif
}