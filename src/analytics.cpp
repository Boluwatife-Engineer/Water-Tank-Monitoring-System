#include "analytics.h"

int highestLevel = 0;
int lowestLevel = 100;
int total = 0;
int count = 0;
int changes = 0;
int prev = -1;

void updateAnalytics(int level){
  if(level > highestLevel) highestLevel = level;
  if(level < lowestLevel) lowestLevel = level;

  total += level;
  count++;

  if(level != prev){
    changes++;
    prev = level;
  }
}

int getHighestLevel(){ return highestLevel; }
int getLowestLevel(){ return lowestLevel; }

float getAverageLevel(){
  if(count == 0) return 0;
  return (float)total / count;
}

int getChangeCount(){ return changes; }