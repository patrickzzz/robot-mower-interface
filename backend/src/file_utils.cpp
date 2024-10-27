#include "file_utils.h"

bool initializeSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount file system");
    return false;
  }
  return true;
}

void showUsageOfSPIFFSFileSystem() {
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();

  logMessage("SPIFFS total: " + String(totalBytes) + " Bytes", 2);
  logMessage("SPIFFS used: " + String(usedBytes) + " Bytes", 2);
  logMessage("SPIFFS free: " + String(totalBytes - usedBytes) + " Bytes", 2);
}

void listSPIFFSFiles() {
  File root = SPIFFS.open("/");
  if (!root) {
    Serial.println("Failed to open root directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    // seems to loop if writing into log file itself..
    Serial.println("File: " + String(file.name()) + " - Size: " + String(file.size()) + " bytes");
    file = root.openNextFile();
  }
}