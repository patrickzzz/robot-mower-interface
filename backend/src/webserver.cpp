#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include "webserver.h"
#include "wifi_utils.h"
#include "logger.h"
#include "mower.h"

// Create Webserver on port 80
AsyncWebServer server(80);

void initializeWebServer() {
  logMessage("Starting HTTP-Server");
  initializeWebserverRoutes();

  server.begin();
  logMessage("HTTP-Server started");
}

void initializeWebserverRoutes() {
// Webserver routes
  server.serveStatic("/", SPIFFS, "/frontend/").setDefaultFile("index.html").setCacheControl("max-age=86400");
  server.serveStatic("/log-messages", SPIFFS, "/log-messages.txt").setCacheControl("no-cache, no-store, must-revalidate");

  server.on("/start", HTTP_POST, [](AsyncWebServerRequest *request) {
    startMower(true);
    request->send(200);
  });

  server.on("/home", HTTP_POST, [](AsyncWebServerRequest *request) {
    sendMowerHome(true);
    request->send(200);
  });

  server.on("/stop", HTTP_POST, [](AsyncWebServerRequest *request) {
    logMessage("Stopping mower", 2);

    pressStopButton(150);

    request->send(200);
  });

  server.on("/lock", HTTP_POST, [](AsyncWebServerRequest *request) {
    if(!isLocked()) {
      lock();
    }
    request->send(200);
  });

  server.on("/unlock", HTTP_POST, [](AsyncWebServerRequest *request) {
    if(isLocked()) {
      unlock();
    }
    request->send(200);
  });

  server.on("/status", HTTP_GET, handleGetStatus);
  server.on("/mowing-plan", HTTP_GET, handleGetMowingPlan);
  server.addHandler(createSetMowingPlanHandler());
  server.on("/wifis", HTTP_GET, handleGetWifis);
  server.addHandler(createSetWifiHandler());
  server.addHandler(createSetDateAndTimeHandler());
}

// handlers
void handleGetStatus(AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(2048);

  // time and date on mower
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    doc["date"] = nullptr;
    doc["time"] = nullptr;
  }else{
    char dateStr[11];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);
    char timeStr[6];
      strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);

      doc["date"] = dateStr;
      doc["time"] = timeStr;
  }

  doc["isCharging"] = isAttachedToCharger();
  doc["isLocked"] = isLocked();
  doc["isEmergency"] = isEmergency();
  doc["isIdle"] = isIdle();
  doc["isAccessPoint"] = getApMode();
  doc["hostname"] = WiFi.getHostname();

  if(getApMode()) {
    doc["ssid"] = getApName();
    doc["ip"] = WiFi.softAPIP().toString();
  }else{
    doc["ssid"] = WiFi.SSID();
    doc["ip"] = WiFi.localIP().toString();
  }

  doc["mowingPlanActive"] = isCurrentMovingPlanActive();

  // send as response
  String responseString;
  serializeJson(doc, responseString);

  // send response with no cache
  AsyncWebServerResponse *response = request->beginResponse(200, "application/json", responseString);
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  request->send(response);
}

void handleGetMowingPlan(AsyncWebServerRequest *request) {
  // check if file exists
  if (!SPIFFS.exists("/mowing_plan.json")) {
    logMessage("Mowing Plan file does not exist", 0);
    request->send(204);
    return;
  }
  // serve static json
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/mowing_plan.json", "application/json");
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  request->send(response);
}

AsyncCallbackJsonWebHandler* createSetMowingPlanHandler() {
  return new AsyncCallbackJsonWebHandler("/mowing-plan", [](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();

    if (jsonObj.containsKey("customMowingPlanActive") && jsonObj.containsKey("days") &&
        jsonObj.containsKey("planTimeStart") && jsonObj.containsKey("planTimeEnd")) {

        MowingPlan plan;

        // Parse the custom mowing plan status
        plan.customMowingPlanActive = jsonObj["customMowingPlanActive"].as<bool>();

        // Parse the days (array of booleans)
        JsonArray daysArray = jsonObj["days"].as<JsonArray>();
        for (int i = 0; i < 7; i++) {
            plan.days[i] = daysArray[i].as<bool>();
        }

        // Parse start and end time
        plan.startTime = jsonObj["planTimeStart"].as<String>();
        plan.endTime = jsonObj["planTimeEnd"].as<String>();

        // Save the mowing plan
        saveMowingPlan(plan);

        // Send success response
        request->send(200);

        // Optional: Check for automatic start or sending the mower home
        checkAutomaticStartOrSendingHomeRequired();
    } else {
        // Send error response if parameters are missing
        request->send(400);
    }
  });
}

void handleGetWifis(AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(2048);
  JsonArray array = doc.to<JsonArray>();

  for (int i = 0; i < getNetworks(); ++i) {
    String wifiSSID = WiFi.SSID(i);
    array.add(wifiSSID);
  }

  String responseString;
  serializeJson(doc, responseString);

  AsyncWebServerResponse *response = request->beginResponse(200, "application/json", responseString);
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  request->send(response);

  asyncScanNetworks();
}

AsyncCallbackJsonWebHandler* createSetWifiHandler() {
    return new AsyncCallbackJsonWebHandler("/wifi", [](AsyncWebServerRequest *request, JsonVariant &json) {
        JsonObject jsonObj = json.as<JsonObject>();

        if (jsonObj.containsKey("ssid") && jsonObj.containsKey("password")) {
            String newSsid = jsonObj["ssid"].as<String>();
            String newPassword = jsonObj["password"].as<String>();

            // check if empty
            if(newSsid.length() == 0 || newPassword.length() == 0) {
                request->send(400);
                return;
            }

            Serial.println("Saving WiFi SSID: " + newSsid);

            saveWifiCredentials(newSsid, newPassword);

            request->send(200);

            delay(2000);
            ESP.restart();
        } else {
            request->send(400, "text/plain", "Missing ssid or password parameter");
        }
    });
}

AsyncCallbackJsonWebHandler* createSetDateAndTimeHandler() {
    return new AsyncCallbackJsonWebHandler("/date-time", [](AsyncWebServerRequest *request, JsonVariant &json) {
        JsonObject jsonObj = json.as<JsonObject>();

        if (jsonObj.containsKey("date") && jsonObj.containsKey("time")) {
            String dateStr = jsonObj["date"].as<String>(); // Format: YYYY-MM-DD
            String timeStr = jsonObj["time"].as<String>(); // Format: HH:MM

            Serial.println("Setting time to: " + dateStr + " " + timeStr);

            int year = dateStr.substring(0, 4).toInt();
            int month = dateStr.substring(5, 7).toInt();
            int day = dateStr.substring(8, 10).toInt();
            int hour = timeStr.substring(0, 2).toInt();
            int minute = timeStr.substring(3, 5).toInt();

            struct tm timeinfo;
            timeinfo.tm_year = year - 1900;
            timeinfo.tm_mon = month - 1;
            timeinfo.tm_mday = day;
            timeinfo.tm_hour = hour;
            timeinfo.tm_min = minute;
            timeinfo.tm_sec = 0;
            timeinfo.tm_isdst = -1;

            time_t t = mktime(&timeinfo);
            struct timeval now = { .tv_sec = t };
            settimeofday(&now, NULL);

            request->send(200, "text/plain", "Time set successfully");
        } else {
            request->send(400, "text/plain", "Missing date or time parameter");
        }
    });
}