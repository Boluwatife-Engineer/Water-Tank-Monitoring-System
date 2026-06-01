#pragma once
#include <Arduino.h>

void initEmail();

bool sendDailyEmail(int highest, int lowest, float average, int changes);