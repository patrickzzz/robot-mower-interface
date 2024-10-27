#ifndef LOGGER_H
#define LOGGER_H

bool initializeLogger();
void logMessage(String text, int debugLevel = 1);
void resetLogFile();

#endif