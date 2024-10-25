#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include "esp_task_wdt.h"

// 0 = no debug (but errors), 1 = normal debug, 2 = more debug (verbose)
int debugLevel = 2;

// Timeserver
const char* ntpServer = "pool.ntp.org";

// Default Wifi-Config
String ssid_default = "Robot Mower";
String password_default = "MowerInterface";
String ssid = "";
String password = "";

// Create Webserver on port 80
AsyncWebServer server(80);
bool apMode = false;
int networks = 0;
String apName;

// Definition of GPIO-Pins
const int pinButtonStart = 22;
const int pinButtonHome = 23;
const int pinButtonStop = 21;
const int pinButtonLock = 17;
const int pinLedCharging = 19;
const int pinLedLocked = 18;
const int pinLedEmergency = 16;
const int pinIdle = 5;

struct MowingPlan {
  bool customMowingPlanActive;
  bool days[7];  // Days (Mo=0, Di=1, ..., So=6)
  String startTime;
  String endTime;
};

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

// Wifi functions
void scanNetworks();
bool loadWifiCredentials();
void saveWifiCredentials(String newSsid, String newPassword);
bool connectToWifi();
void startAccessPoint();
void getAccessPointNameForDevice();

// Route functions
void handleGetStatus(AsyncWebServerRequest *request);
void handleGetMowingPlan(AsyncWebServerRequest *request);
AsyncCallbackJsonWebHandler* createSetMowingPlanHandler();
void handleGetWifis(AsyncWebServerRequest *request);
void handleSetWifi(AsyncWebServerRequest *request);
AsyncCallbackJsonWebHandler* createSetDateAndTimeHandler();

// Mower functions
void pressButton(int pin, int duration = 150, bool holdStopButtonPressed = false);
void pressStopButton(int releaseAfter = 0);
void saveMowingPlan(MowingPlan plan);
MowingPlan loadMowingPlan();
void checkAutomaticStartOrSendingHomeRequired();
bool isMowingTime();
void checkStateChangeInDockingOrOutside();
void startMower(bool isManual = false);
void sendMowerHome(bool isManual = false);
bool checkCommandIsNotRepeatedTooEarly(String command);
void unlock();
bool isAttachedToCharger();
bool isLocked();
bool isEmergency();
bool isIdle();

// Debug functions
void logMessage(String text, int debugLevel = 1);
void resetLogFile();

void setup() {
  esp_task_wdt_init(30, false); // Wait time in seconds

  Serial.begin(115200);

  // Initialize Spiffs
  if (!SPIFFS.begin(true)) {
      Serial.println("SPIFFS Mount Failed");
      return;
  }

  resetLogFile();
  logMessage("Starting Robot Mower Interface", 1);

  // Initialize GPIO-Pins
  // Start
  pinMode(pinButtonStart, OUTPUT_OPEN_DRAIN);
  digitalWrite(pinButtonStart, HIGH);

  // Home
  pinMode(pinButtonHome, OUTPUT_OPEN_DRAIN);
  digitalWrite(pinButtonHome, HIGH);

  // Stop
  pinMode(pinButtonStop, OUTPUT_OPEN_DRAIN);
  digitalWrite(pinButtonStop, LOW);

  // Locked
  pinMode(pinButtonLock, OUTPUT_OPEN_DRAIN);
  digitalWrite(pinButtonLock, HIGH);

  // Check LEDs
  pinMode(pinLedCharging, INPUT);
  pinMode(pinLedLocked, INPUT);
  pinMode(pinLedEmergency, INPUT);

  // Check Power on Pin5 (Idle)
  pinMode(pinIdle, INPUT);

  // Try to load wifi credentials or start access point
  if (loadWifiCredentials() && connectToWifi()) {
    logMessage("Trying to synchronize time from NTP server", 1);
    configTime(0, 0, ntpServer);
  }else{
    startAccessPoint();
  }

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
    logMessage("Locking mower", 2);
    pressButton(pinButtonLock);
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
  server.on("/wifi", HTTP_POST, handleSetWifi);
  server.addHandler(createSetDateAndTimeHandler());

  server.begin();
  logMessage("HTTP-Server started");

  // Load Mowing Plan
  currentMowingPlan = loadMowingPlan();
}

void loop() {
  // check every minute if actions are required
  checkAutomaticStartOrSendingHomeRequired();
  checkStateChangeInDockingOrOutside();
  delay(60000);
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

// Scan for available networks
void scanNetworks() {
  logMessage("Scanning Wifis..");
  networks = WiFi.scanNetworks();
  logMessage("Scan done", 1);
  if (networks == 0) {
    logMessage("No networks found", 1);
  } else {
    logMessage(String(networks) + " networks found", 1);
    for (int i = 0; i < networks; ++i) {
      logMessage(WiFi.SSID(i), 1);
    }
  }
}

// Connect to Wifi
bool connectToWifi() {
  WiFi.begin(ssid, password);
  logMessage("Try to connect with Wifi: ", 1);
  logMessage(ssid, 1);
  logMessage("Password: ", 1);
  String pwString = "";
  for(int i = 0; i < password.length(); i++) {
    pwString += "*";
  }
  logMessage(pwString, 1);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    logMessage("Connected!", 1);
    // ToDo: Set Hostname
    // WiFi.hostname("MyESP8266");
    // Serial.println(  );
    logMessage("Webinterface available at: http://"+ WiFi.localIP().toString());
    return true;
  } else {
    logMessage("Wifi connection failed.", 0);
    return false;
  }
}

// Start Access Point
void startAccessPoint() {
  logMessage("Starting Access Point", 1);
  getAccessPointNameForDevice();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName, password_default);
  apMode = true;
  logMessage("Access Point started: ", 1);
  logMessage(apName, 1);
  logMessage(password_default, 1);
  logMessage("Webinterface available at: http://" + WiFi.softAPIP().toString(), 1);
}

// generate Access Point Name for Device
// keep this in a .txt file, to use the same after reboot again
void getAccessPointNameForDevice() {
  if(SPIFFS.exists("/ap.txt")) {
    File file = SPIFFS.open("/ap.txt", "r");
    if (file) {
      apName = file.readStringUntil('\n');
      file.close();
      if(apName.length() > 0) {
        return;
      }
    }
  }

  apName = ssid_default + " " + String(random(0xffff), HEX);
  File file = SPIFFS.open("/ap.txt", "w");
  if (file) {
    file.println(apName);
    file.close();
  }
}

// Load Wifi Credentials from SPIFFS
bool loadWifiCredentials() {
  if (SPIFFS.exists("/wifi.txt")) {
    File file = SPIFFS.open("/wifi.txt", "r");
    if (file) {
      String readSsid = file.readStringUntil('\n');
      readSsid.trim();
      ssid = readSsid;

      String readPassword = file.readStringUntil('\n');
      readPassword.trim();
      password = readPassword;

      file.close();
      if(ssid.length() > 0 && password.length() > 0) {
        return true;
      }
    }
  }
  return false;
}

// Save Wifi Credentials to SPIFFS
void saveWifiCredentials(String newSsid, String newPassword) {
  File file = SPIFFS.open("/wifi.txt", "w");
  if (file) {
    file.println(newSsid);
    file.println(newPassword);
    file.close();
  } else {
    logMessage("Failed to open file for writing: /wifi.txt", 0);
  }
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
  doc["isAccessPoint"] = apMode;
  doc["ssid"] = WiFi.SSID();
  doc["ip"] = WiFi.localIP().toString();
  doc["mowingPlanActive"] = currentMowingPlan.customMowingPlanActive;

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
  // Initialize Wifi
  scanNetworks();

  DynamicJsonDocument doc(2048);
  JsonArray array = doc.to<JsonArray>();

  for (int i = 0; i < networks; ++i) {
    String wifiSSID = WiFi.SSID(i);
    array.add(wifiSSID);
  }

  String responseString;
  serializeJson(doc, responseString);

  AsyncWebServerResponse *response = request->beginResponse(200, "application/json", responseString);
  response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  request->send(response);
}

void handleSetWifi(AsyncWebServerRequest *request) {
  String newSsid = request->arg("ssid");
  String newPassword = request->arg("password");
  saveWifiCredentials(newSsid, newPassword);

  request->send(200);

  delay(2000);
  ESP.restart();
}

AsyncCallbackJsonWebHandler* createSetDateAndTimeHandler() {
    return new AsyncCallbackJsonWebHandler("/date-time", [](AsyncWebServerRequest *request, JsonVariant &json) {
        JsonObject jsonObj = json.as<JsonObject>();

        if (jsonObj.containsKey("date") && jsonObj.containsKey("time")) {
            String dateStr = jsonObj["date"].as<String>(); // Format: YYYY-MM-DD
            String timeStr = jsonObj["time"].as<String>(); // Format: HH:MM

            Serial.println("Setting time to: " + dateStr + " " + timeStr);

            // Teile des Datums und der Uhrzeit extrahieren
            int year = dateStr.substring(0, 4).toInt();
            int month = dateStr.substring(5, 7).toInt();
            int day = dateStr.substring(8, 10).toInt();
            int hour = timeStr.substring(0, 2).toInt();
            int minute = timeStr.substring(3, 5).toInt();

            // Erstellen einer tm-Struktur
            struct tm timeinfo;
            timeinfo.tm_year = year - 1900;
            timeinfo.tm_mon = month - 1;
            timeinfo.tm_mday = day;
            timeinfo.tm_hour = hour;
            timeinfo.tm_min = minute;
            timeinfo.tm_sec = 0;
            timeinfo.tm_isdst = -1;

            // Systemzeit setzen
            time_t t = mktime(&timeinfo);
            struct timeval now = { .tv_sec = t };
            settimeofday(&now, NULL);

            request->send(200, "text/plain", "Time set successfully");
        } else {
            request->send(400, "text/plain", "Missing date or time parameter");
        }
    });
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

void logMessage(String text, int debugLevelOfMessage) {
  if(debugLevelOfMessage > debugLevel) {
    return;
  }

  String timeString = "";

  // get current time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    timeString = "[Time not available] ";
  }else{
    // create a string with the current time
    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%a, %y/%m/%d %H:%M:%S", &timeinfo);
    timeString = "[" + String(timeStr) + "] ";
  }

  // print the text and the current time to serial
  Serial.println(timeString + text);

  // print the text and the current time to a text file
  File file = SPIFFS.open("/log-messages.txt", "a");
  if(file.size() > 50000) {  // 5 kb
    // just reset the file, if it is bigger than 5 kb
    file.close();
    resetLogFile();
    file = SPIFFS.open("/log-messages.txt", "w");
    file.println(timeString + "Logfile has been truncated..");
  }
  if (file) {
    file.println(timeString + text);
    file.close();
  } else {
    Serial.println(timeString + "Failed to open file for writing: /log-messages.txt");
  }
}

void resetLogFile() {
  logMessage("Truncating log file", 0);

  // clear files by opening them with "w" flag
  File file = SPIFFS.open("/log-messages.txt", "w");
  file.close();
}