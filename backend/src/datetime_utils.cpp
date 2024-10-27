#include <Arduino.h>
#include "logger.h"

void syncNTPTime() {
  logMessage("Syncing NTP time", 2);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
}