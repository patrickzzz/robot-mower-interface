#ifndef WEBSERVER_UTILS_H
#define WEBSERVER_UTILS_H

#include <Arduino.h>
#include <ESPAsyncWebserver.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

void initializeWebServer();
void initializeWebserverRoutes();

void handleGetStatus(AsyncWebServerRequest *request);
void handleGetMowingPlan(AsyncWebServerRequest *request);
AsyncCallbackJsonWebHandler* createSetMowingPlanHandler();
void handleGetWifis(AsyncWebServerRequest *request);
AsyncCallbackJsonWebHandler* createSetWifiHandler();
AsyncCallbackJsonWebHandler* createSetDateAndTimeHandler();

#endif