#include <SPIFFS.h>
#include <WiFi.h>
#include "esp_task_wdt.h"
#include "wifi_utils.h"
#include "logger.h"
#include "file_utils.h"
#include "datetime_utils.h"
#include "pins.h"
#include "mower.h"
#include "webserver.h"

int loopCounter = 0;

void setup() {
  // for now a higher timeout, for some possible longer requests on webserver resources.
  // but as content is gzipped, it should be fine
  esp_task_wdt_init(60, false); // Wait time in seconds

  Serial.begin(115200);

  if (!initializeSPIFFS()) {
      return;
  }

  initializeLogger();
  listSPIFFSFiles();
  showUsageOfSPIFFSFileSystem();

  logMessage("Update works!!", 2);

  logMessage("Starting Robot Mower Interface", 1);
  setupPins();

  setDefaultHostname();
  if (loadWifiCredentials() && connectToWifi()) {
    syncNTPTime();
  }else{
    startAccessPoint();
  }

  initializeWebServer();

  asyncScanNetworks();
}

void loop() {
  // do every 10 seconds
  checkAsyncScanNetworksUpdate();

  // do every 30 seconds
  if(loopCounter == 3 || loopCounter == 0) {
    reconnectToWifiIfNeeded();
  }

  // do every 60 seconds
  if(loopCounter == 0)  {
    checkAutomaticStartOrSendingHomeRequired();
    checkStateChangeInDockingOrOutside();
  }

  loopCounter++;
  if(loopCounter >= 6) {
    loopCounter = 0;
  }

  delay(10000);
}