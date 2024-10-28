#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <Arduino.h>

// may change these to struct
bool getApMode();
String getApName();
int getNetworks();

void setDefaultHostname();
void scanNetworks();
void asyncScanNetworks();
void checkAsyncScanNetworksUpdate();
String getNetworksJsonArray();
bool loadWifiCredentials();
void saveWifiCredentials(String newSsid, String newPassword);
bool connectToWifi();
void reconnectToWifiIfNeeded();
void startAccessPoint();
void getAccessPointNameForDevice();
void removeOldestEntry();

#endif