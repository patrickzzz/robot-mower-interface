#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include "logger.h"

// 0 = no debug (but errors), 1 = normal debug, 2 = more debug (verbose)
File logFile;
bool logResetInProgress = false;
int debugLevel = 2;

bool initializeLogger() {
  if (SPIFFS.exists("/log-messages.txt")) {
    SPIFFS.remove("/log-messages.txt");
  }

  logFile = SPIFFS.open("/log-messages.txt", "w");
  if (logFile) {
    logMessage("Logfile created.", 2);
    return true;
  } else {
    Serial.println("Failed to create log file");
    return false;
  }
}

void logMessage(String text, int debugLevelOfMessage) {
  if(debugLevelOfMessage > debugLevel) {
    return;
  }

  String timeString = "";
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    timeString = "[Time unavailable] ";
  } else {
    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%a, %y/%m/%d %H:%M:%S", &timeinfo);
    timeString = "[" + String(timeStr) + "] ";
  }

  Serial.println(timeString + text);

  if (logFile && !logResetInProgress) {
    if (logFile.size() > 50000) {
      Serial.println(timeString + "Logfile is too big, resetting it");

      logResetInProgress = true;
      resetLogFile();
      logResetInProgress = false;

      if(!logFile) {
        return;
      }
    }
    logFile.println(timeString + text);
    logFile.flush();
  }
}

void resetLogFile() {
  Serial.println("Resetting log file");
  if (logFile) {
    logFile.close();
  }

  if (SPIFFS.exists("/log-messages.txt")) {
    SPIFFS.remove("/log-messages.txt");
  }

  logFile = SPIFFS.open("/log-messages.txt", "w");
  if (logFile) {
    Serial.println("Logfile has been resetted");
  } else {
    Serial.println("Failed to reset log file");
  }
}