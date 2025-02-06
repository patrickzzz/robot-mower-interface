/*
 * AH20241203 infos for @patrickzzz:
 * - Partitioned with 2*OTA "Safe update mode" see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html
 * - Followed Google C++ Sytle Guide see https://google.github.io/styleguide/cppguide.html#Naming except for "k" prefix for constants which I don't liked to follow ;-)
 */

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <string.h>

#include "LedSequencer.hpp"
#include "config.h"
#include "pins.h"
#include "YFComms/CoverUIController.hpp"
#include "MyButtonHandler.hpp"
#include "mainboards/mainboard_driver_om.hpp"
#include "YFComms/LED.hpp"

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
    led_grn_seq.blink({.limit_blink_cycles = 3, .fulfill = true});  // Default = 200ms ON, 200ms OFF
    led_red_seq.blink({.limit_blink_cycles = 3, .fulfill = true});  // Default = 200ms ON, 200ms OFF

    // @patrickzzz WiFi & WebServer init should start somewhere here.
    // Mainly because WiFi doesn't depend on any of the following peripherals.
    // But backend should check for sys_health if all peripherals are healthy and report in frontend if not!
    // Example how you could fail with a mandatory peripheral (i.e. WiFi or Webserver) error
    if (esp_err_t ret = ESP_OK != ESP_OK) {  // Replace first ESP_OK with your WiFi and/or Webserver init which return with a esp_err_t
        ESP_LOGE(TAG, "Example failed with error: %s", esp_err_to_name(ret));
        LED_SEQ_FATAL_ERROR;
        return;  // Other tasks will still run!
    }

#ifdef DEV_AH
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
#endif

#ifdef DEV_PK
    // create coverUIController for specific model
    YFComms::CoverUIController coverUIController("500Classic");

    // create structs for uart configs
    YFComms::UARTConfig uartConfigCoverUI = YFComms::UARTConfig{.uartPort = UART_NUM_1, .rxPin = pinUartR6BRx, .txPin = pinUartR6BTx};
    YFComms::UARTConfig uartConfigHardwareMainboard = YFComms::UARTConfig{.uartPort = UART_NUM_2, .rxPin = pinUartR6ARx, .txPin = pinUartR6ATx};

    // set configs
    coverUIController.setUARTConfigCoverUI(uartConfigCoverUI);
    coverUIController.setUARTConfigHardwareMainboard(uartConfigHardwareMainboard);

    // set mainboard type
    coverUIController.setMainboardType(YFComms::MainboardType::YF_HARDWARE);
    //coverUIController.setMainboardType(YFComms::MainboardType::YF_VIRTUAL);

    // initialize coverUIController
    coverUIController.initialize();

    // add Button/Hall Event Listener
    MyButtonHandler myButtonHandler;
    coverUIController.getButtonState().setListener(&myButtonHandler);

    // enable leds for first test..
    coverUIController.setAllLEDsMode(YFComms::led::Modes::FLASH_FAST);

    vTaskDelay(4500 / portTICK_PERIOD_MS);  // delay(1000)
    coverUIController.setAllLEDsMode(YFComms::led::Modes::OFF);
    coverUIController.setLEDMode(YFComms::led::Names::LIFTED, YFComms::led::Modes::ON);
    coverUIController.setLEDMode(YFComms::led::Names::SIGNAL, YFComms::led::Modes::ON);
    coverUIController.setLEDMode(YFComms::led::Names::HOURS_TWO, YFComms::led::Modes::ON);
    coverUIController.setLEDMode(YFComms::led::Names::BATTERY_LOW, YFComms::led::Modes::ON);
    coverUIController.setLEDMode(YFComms::led::Names::CHARGING, YFComms::led::Modes::FLASH_SLOW);

    // switch to 500c and update led states there as well..
 //   coverUIController.changeModel("500Classic");
#endif

    ESP_LOGI(TAG, "Start");
#ifdef DEV_PK
    while (1) {
        led_grn_seq.blink({.on_ms = 20, .limit_blink_cycles = 1, .fulfill = true});
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // delay(1000)
    }
#endif

#ifdef DEV_AH
    while (1) {
        // app_main task "alive" flash
        led_grn_seq.blink({.on_ms = 20, .limit_blink_cycles = 1, .fulfill = true});
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // delay(1000)

        // Let's output some task/stack critical values to be monitored during development
        ESP_LOGI(TAG, "Task high water mark (free stack words) of: LEDs_Task %d",
                 uxTaskGetStackHighWaterMark(leds_task_handle));
    }
#endif
}