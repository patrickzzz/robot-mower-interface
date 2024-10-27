#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <SPIFFS.h>
#include "logger.h"

bool initializeSPIFFS();
void showUsageOfSPIFFSFileSystem();
void listSPIFFSFiles();
#endif