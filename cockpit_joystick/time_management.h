#include "Arduino.h"

#ifndef TimeManagement_h
#define TimeManagement_h

// -----------------------------------------------------------------------------------------------
// Time manageing functions

public class TimeManagement
{
  unsigned long m_startTime = 0;
  unsigned long m_cyclePeriod = 16666; // 60 Hz period

  // Benchmarking
  unsigned long m_accumulatedDuration = 0;
  unsigned long m_maxDuration = 0;
  unsigned long m_minDuration = 0;
  unsigned long m_benchmarkCount = 0;

  TimeManagement();

  void sampleTime(void);
  unsigned long findCycleDuration(void);
  unsigned long getDelay(void);
  unsigned doPeriodDelay(void);
  void resetBenchmarking(void);

  // Returns true is benchmark report is to be produced
  bool benchmarkHandleDuration(unsigned long duration);

};

#endif
