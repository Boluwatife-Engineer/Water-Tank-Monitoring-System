#ifndef EMAIL_H
#define EMAIL_H

#include <Arduino.h>

void initEmail();

bool sendDailyEmail(int highest, int lowest, float average, int changes);

#endif