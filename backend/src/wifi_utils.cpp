#include <WiFi.h>
#include <SPIFFS.h>
#include "wifi_utils.h"
#include "logger.h"
#include <WiFiMulti.h>
#include <ArduinoJson.h>

// Default Wifi-Config
String hostname_default = "robotmower";
String ssid_default = "Robot Mower";
String password_default = "MowerInterface";
bool onceConnectedToWifi = false;
bool apMode = false;
String apName;
int networks = 0;

int scanInterval = 25000;
unsigned long lastScanTime = 0;
// json doc with networks as array
DynamicJsonDocument networksDoc(2048);

WiFiMulti wifiMulti;

bool getApMode() {
    return apMode;
}

String getApName() {
    return apName;
}

int getNetworks() {
    return networks;
}

void setDefaultHostname() {
  WiFi.setHostname(hostname_default.c_str());
}

// Scan for available networks
void scanNetworks() {
    logMessage("Wifi scan started");
    networks = WiFi.scanNetworks();
    logMessage("Async WiFi scan completed: " + String(networks) + " networks found.");
    if (networks > 0) {
      for (int i = 0; i < networks; ++i) {
          logMessage("Network " + String(i + 1) + ": " + WiFi.SSID(i), 1);
      }
    }
}

// Asynchronous Scan for available networks
void asyncScanNetworks() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastScanTime >= scanInterval) {
    logMessage("Async Wifi scan started..");
    lastScanTime = currentMillis;
    WiFi.scanNetworks(true);
  }else{
    logMessage("Async Wifi scan already in progress..");
  }
}

void checkAsyncScanNetworksUpdate() {
  int n = WiFi.scanComplete();
  if(n > 0 && n != networks) {
    networks = n;

    // create new doc here, and set networksDoc when this doc is ready
    DynamicJsonDocument doc(2048);
    JsonArray array = doc.to<JsonArray>();

    logMessage("Async WiFi scan completed: " + String(networks) + " networks found.");
    for (int i = 0; i < networks; i++) {
      String wifiSSID = WiFi.SSID(i);
      logMessage("Network " + String(i + 1) + ": " + wifiSSID, 1);
      array.add(wifiSSID);
    }
    networksDoc.clear();
    networksDoc = doc;
  }
}

String getNetworksJsonArray() {
  // if no data, then return empty array, otherwise return the networks
  if(networksDoc.size() == 0) {
    return "[]";
  }

  String responseString;
  serializeJson(networksDoc, responseString);

  return responseString;
}

// Check for duplicate SSID and Password
bool checkDuplicates(String ssid, String password) {
    if (SPIFFS.exists("/wifi.txt")) {
        File file = SPIFFS.open("/wifi.txt", "r");
        if (file) {
            while (file.available()) {
                String line = file.readStringUntil('\n');
                line.trim();
                if (line.length() > 0) {
                    DynamicJsonDocument doc(200); // Anpassung der Größe nach Bedarf
                    DeserializationError error = deserializeJson(doc, line);
                    if (!error) {
                        String existingSsid = doc["ssid"];
                        String existingPassword = doc["password"];
                        // Check for duplicates
                        if (existingSsid == ssid) {
                            if (existingPassword == password) {
                                logMessage("Duplicate entry found: " + ssid + " with the same password.", 0);
                                return true; // duplicate in SSID+Pass
                            } else {
                                logMessage("SSID already exists with a different password: " + ssid, 1);
                                return false; // SSID exists, different password
                            }
                        }
                    }
                }
            }
            file.close();
        }
    }
    return false;
}

// Connect to Wifi
bool connectToWifi() {
    logMessage("Trying to connect to the best available Wifi...", 1);
    if (wifiMulti.run() == WL_CONNECTED) {
        logMessage("Connected to WiFi!", 1);
        logMessage("Webinterface available at: http://" + WiFi.localIP().toString());
        onceConnectedToWifi = true;
        return true;
    } else {
        logMessage("Wifi connection failed.", 0);
        return false;
    }
}

void reconnectToWifiIfNeeded() {
  if(WiFi.status() != WL_CONNECTED && apMode == false && onceConnectedToWifi == true) {
    logMessage("Wifi connection lost, trying to reconnect..", 0);
    connectToWifi();
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

// Generate Access Point Name for Device
void getAccessPointNameForDevice() {
    if (SPIFFS.exists("/ap.txt")) {
        File file = SPIFFS.open("/ap.txt", "r");
        if (file) {
            apName = file.readStringUntil('\n');
            file.close();
            if (apName.length() > 0) {
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
  Serial.println("Loading Wifi Credentials from SPIFFS");
    if (SPIFFS.exists("/wifi.txt")) {
        File file = SPIFFS.open("/wifi.txt", "r");
        if (file) {
            while (file.available()) {
                String line = file.readStringUntil('\n');
                Serial.println("Line: " + line);
                line.trim();
                if (line.length() > 0) {
                    DynamicJsonDocument doc(200); // Anpassung der Größe nach Bedarf
                    DeserializationError error = deserializeJson(doc, line);
                    if (!error) {
                        String readSsid = doc["ssid"];
                        String readPassword = doc["password"];
                        wifiMulti.addAP(readSsid.c_str(), readPassword.c_str());
                    } else {
                        logMessage("Failed to parse JSON: " + String(error.c_str()), 0);
                    }
                }
            }
            file.close();
            return true;
        }
    }
    return false;
}

// Save Wifi Credentials to SPIFFS
// ToDo: return bool for success
void saveWifiCredentials(String newSsid, String newPassword) {
    if (checkDuplicates(newSsid, newPassword)) {
        logMessage("Entry already exists, not saving: " + newSsid, 0);
        return;
    }

    File file = SPIFFS.open("/wifi.txt", "a"); // Append Mode
    if (file) {
        DynamicJsonDocument doc(200);
        doc["ssid"] = newSsid;
        doc["password"] = newPassword;

        String output;
        serializeJson(doc, output);
        file.println(output);
        file.close();
        logMessage("Saved new WiFi credentials: " + newSsid, 1);

        removeOldestEntry();
    } else {
        logMessage("Failed to open file for writing: /wifi.txt", 0);
    }
}

// Remove the oldest entry if there are more than 10 entries
void removeOldestEntry() {
    File file = SPIFFS.open("/wifi.txt", "r+"); // Read and Write Mode
    if (file) {
        String lines[11];
        int count = 0;

        while (file.available() && count < 11) {
            lines[count] = file.readStringUntil('\n');
            lines[count].trim();
            if (lines[count].length() > 0) {
                count++;
            }
        }

        // Delete oldest, if more than 10 wifis saved
        if (count > 10) {
            file.close();

            file = SPIFFS.open("/wifi.txt", "w"); // Clear the file for rewriting
            if (file) {
                for (int i = 1; i < count; i++) {
                    file.println(lines[i]);
                }
                logMessage("Oldest entry removed, total entries: " + String(count - 1), 1);
            }
            file.close();
        } else {
            file.close();
        }
    }
}