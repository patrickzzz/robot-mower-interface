#ifndef MOWER_H
#define MOWER_H

#include <Arduino.h>

struct MowingPlan {
  bool customMowingPlanActive;
  bool days[7];  // Days (Mo=0, Di=1, ..., So=6)
  String startTime;
  String endTime;
};

bool isCurrentMovingPlanActive();
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
void lock();
bool isAttachedToCharger();
bool isLocked();
bool isEmergency();
bool isIdle();

#endif