#include "analytics.h"

int highestLevel = 0;

int lowestLevel = 100;

int totalLevel = 0;

int readingCount = 0;

int changes = 0;

int previousLevel = -1;



void updateAnalytics(int level)
{
  if (level > highestLevel)
  {
    highestLevel = level;
  }

  if (level < lowestLevel)
  {
    lowestLevel = level;
  }

  totalLevel += level;

  readingCount++;

  if (level != previousLevel)
  {
    changes++;

    previousLevel = level;
  }
}



int getHighestLevel()
{
  return highestLevel;
}



int getLowestLevel()
{
  return lowestLevel;
}



float getAverageLevel()
{
  if (readingCount == 0)
  {
    return 0;
  }

  return (float) totalLevel / readingCount;
}



int getChangeCount()
{
  return changes;
}