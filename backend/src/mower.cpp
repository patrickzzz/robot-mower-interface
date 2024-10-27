#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "mower.h"
#include "logger.h"
#include "pins.h"

MowingPlan currentMowingPlan;
bool mowerWasStartedManually = false;
String lastManualStop = "";
String stateInDockingOrOutside = "";

struct LastAutomaticCommand {
  String command;
  time_t timestamp;
  int amountRetries;
};

LastAutomaticCommand lastAutomaticCommand;

bool isCurrentMovingPlanActive() {
  return currentMowingPlan.customMowingPlanActive;
}

void checkAutomaticStartOrSendingHomeRequired() {
  logMessage("Checking automatic start or sending home required", 2);

  // no mowing, if custom mowing plan is not active
  if(!currentMowingPlan.customMowingPlanActive) {
    logMessage("No custom mowing plan active", 2);
    return;
  }

  // if mower is outside and time is later than end time, stop mower
  // get isMowingTime
  if(!isMowingTime()) {
    if(!isIdle() && !isAttachedToCharger()) {
      // only do this, if its not a manual start
      if(!mowerWasStartedManually) {
        if(checkCommandIsNotRepeatedTooEarly("stop")) {
          logMessage("Stopping mower, because its not mowing time", 2);
          sendMowerHome();
        }
      }
    }
  }else{
    if(isIdle() || isAttachedToCharger()) {
      if(checkCommandIsNotRepeatedTooEarly("start")) {
        logMessage("Starting mower, because its mowing time", 2);
        startMower();
      }
    }
  }
}

bool checkCommandIsNotRepeatedTooEarly(String command) {
  // check if last command was the same, and it was only like < 5 minutes ago, then ignore the command
  if(lastAutomaticCommand.command == command) {
    if((time(0) - lastAutomaticCommand.timestamp) < 300) {
      logMessage("Command was already sent less than 5 minutes ago, so ignore the command: " + command, 2);
      // ignore retries for 5 minutes
      return false;
    }

    if(lastAutomaticCommand.amountRetries >= 5) {
      // if last command was the same, and it was less than 5 minutes ago, and it was already tried 3 times, then ignore the command
      logMessage("Command was retried 5 times, so ignore the command: "+ command, 2);
      return false;
    }

    lastAutomaticCommand.amountRetries++;
    lastAutomaticCommand.timestamp = time(0);
    return true;
  }

  lastAutomaticCommand.command = command;
  lastAutomaticCommand.timestamp = time(0);
  lastAutomaticCommand.amountRetries = 0;

  return true;
}

void checkStateChangeInDockingOrOutside() {
  if(isIdle()) {
    return;
  }
  if(isAttachedToCharger()) {
    if(stateInDockingOrOutside == "IN DOCKING") {
      return;
    }
    stateInDockingOrOutside = "IN DOCKING";
    if(currentMowingPlan.customMowingPlanActive) {
      // after coming home:
      // press stop button, if mower is in docking station and custom mowing plan is active
      // this will avoid, that the mower starts according to the own logic ~24hours later
      pressStopButton(150);
    }
  }else{
    if(stateInDockingOrOutside == "OUTSIDE") {
      return;
    }
    stateInDockingOrOutside = "OUTSIDE";
  }
}

bool isMowingTime() {
  logMessage("Checking if its mowing time", 2);

  // check if its mowing time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    logMessage("Failed to obtain time", 0);
    return false;
  }

  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;

  String dateString = String(timeinfo.tm_year) + "-" +
                      String(timeinfo.tm_mon + 1) + "-" +
                      String(timeinfo.tm_mday);

  if(lastManualStop == dateString) {
    logMessage("Mower was stopped manually today, so no mowing time today anymore", 2);
    return false;
  }

  int weekdayIndex = (timeinfo.tm_wday + 6) % 7; // change sunday 0 to monday 0
  if(currentMowingPlan.days[weekdayIndex]) {
    if(currentHour >= currentMowingPlan.startTime.substring(0, 2).toInt() && currentMinute >= currentMowingPlan.startTime.substring(3, 5).toInt()) {
      if(currentHour < currentMowingPlan.endTime.substring(0, 2).toInt() ||
        currentHour == currentMowingPlan.endTime.substring(0, 2).toInt() && currentMinute <= currentMowingPlan.endTime.substring(3, 5).toInt()) {
        logMessage("Mowing time", 2);
        return true;
      }
    }
  }

  logMessage("Not mowing time", 2);
  return false;
}


void pressButton(int pin, int duration, bool holdStopButtonPressed) {
  // log message, which button is pressed, make string from pin
  logMessage("Pressing button: " + String(pin), 2);

  if(holdStopButtonPressed) {
    pressStopButton(0);
    delay(150);
  }

  digitalWrite(pin, LOW);
  delay(duration);
  digitalWrite(pin, HIGH);

  if(holdStopButtonPressed) {
    digitalWrite(pinButtonStop, LOW);
  }
}

void pressStopButton(int releaseAfter) {
  digitalWrite(pinButtonStop, HIGH);
  if(releaseAfter > 0) {
    delay(releaseAfter);
    digitalWrite(pinButtonStop, LOW);
  }
}

void saveMowingPlan(MowingPlan plan) {
  File file = SPIFFS.open("/mowing_plan.json", "w");
  if (!file) {
    logMessage("Failed to open file for writing: /mowing_plan.json", 0);
    return;
  }

  StaticJsonDocument<256> doc;
  doc["customMowingPlanActive"] = plan.customMowingPlanActive;

  JsonArray days = doc.createNestedArray("days");
  for (int i = 0; i < 7; i++) {
    days.add(plan.days[i]);
  }

  doc["startTime"] = plan.startTime;
  doc["endTime"] = plan.endTime;

  if (serializeJson(doc, file) == 0) {
    logMessage("Failed to write to file: /mowing_plan.json", 0);
  }

  file.close();

  currentMowingPlan = loadMowingPlan();
}

MowingPlan loadMowingPlan() {
  MowingPlan plan;

  File file = SPIFFS.open("/mowing_plan.json", "r");
  if (!file) {
    logMessage("Failed to open file for reading", 0);
    return plan;
  }

  // log file content
  String fileContent = file.readString();
  logMessage("Mowing Plan: " + fileContent, 2);

  // set file position to start
  file.seek(0);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    logMessage("Failed to read file /mowing_plan.json, using default settings", 0);
  } else {
    plan.customMowingPlanActive = doc["customMowingPlanActive"];

    for (int i = 0; i < 7; i++) {
      plan.days[i] = doc["days"][i];
    }

    plan.startTime = doc["startTime"].as<String>();
    plan.endTime = doc["endTime"].as<String>();
  }

  file.close();
  return plan;
}

void startMower(bool isManual) {
  logMessage("Starting mower", 2);
  lastManualStop = "";
  mowerWasStartedManually = isManual;
  if(isLocked()) {
    unlock();
  }

  pressButton(pinButtonStart, 150, true);
}

void sendMowerHome(bool isManual) {
  logMessage("Sending mower home", 2);
  if(isManual) {
    // set current day as string to lastManualStop, format YYYY-MM-DD
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char dateStr[11];
      strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
      lastManualStop = String(dateStr);
    }
  }
  if(isLocked()) {
    unlock();
  }

  pressButton(pinButtonHome, 150, true);
}

void unlock() {
  logMessage("Unlocking mower", 2);
  // press 4 times on unlock button
  for(int i = 0; i < 4; i++) {
    pressButton(pinButtonLock);
    delay(100);
  }
}

void lock() {
  logMessage("Locking mower", 2);
  pressButton(pinButtonLock);
}

bool isAttachedToCharger() {
  // check every 50ms if pin is high, until 750ms
  for(int i = 0; i < 15; i++) {
    if(digitalRead(pinLedCharging) == HIGH) {
      return true;
    }
    delay(50);
  }

  return false;
}

bool isLocked() {
  // check every 50ms if pin is high, until 750ms
  for(int i = 0; i < 15; i++) {
    if(digitalRead(pinLedLocked) == HIGH) {
      return true;
    }
    delay(50);
  }

  return false;
}

bool isEmergency() {
  return digitalRead(pinLedEmergency) == HIGH;
}

bool isIdle() {
  return digitalRead(pinIdle) != HIGH;
}