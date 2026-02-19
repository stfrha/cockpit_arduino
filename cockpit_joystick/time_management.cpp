#include "cockpit_joystick_common.h"


// ------------------------------------------------------------------------
// Time functions


TimeManagement::TimeManagement()
{
  m_startTime = 0;
  m_cyclePeriod = 16666; // 60 Hz period
  m_accumulatedDuration = 0;
  m_maxDuration = 0;
  m_minDuration = 0;
  m_benchmarkCount = 0;

  resetBenchmarking();
}


void TimeManagement::sampleTime(void)
{
  m_startTime = micros();
}

unsigned long TimeManagement::findCycleDuration(void)
{
  unsigned long now = micros();

  if (now < m_startTime)
  {
    // We have a wrap, unwrap it
    return 4294967295 - m_startTime + now;
  }

  return now - m_startTime;
}

unsigned long TimeManagement::getDelay(void)
{
  unsigned long duration = findCycleDuration();

  benchmarkHandleDuration(duration);

  if (duration > m_cyclePeriod)
  {
    return 0;
  }

  return m_cyclePeriod - duration;
}

unsigned TimeManagement::doPeriodDelay(void)
{
  delayMicroseconds(getDelay());
}

void TimeManagement::resetBenchmarking(void)
{
  m_accumulatedDuration = 0;
  m_maxDuration = 0;
  m_minDuration = 4294967295;
  m_benchmarkCount = 0;
}

bool TimeManagement::benchmarkHandleDuration(unsigned long duration)
{
  m_accumulatedDuration += duration;

  if (duration > m_maxDuration)
  {
    m_maxDuration = duration;
  }

  if (duration < m_minDuration)
  {
    m_minDuration = duration;
  }

  m_benchmarkCount++;

  if (m_benchmarkCount > 500)
  {
    // Report benchmark and then reset
    Serial1.print("Benchmark duration report! avg: ");
    Serial1.print(m_accumulatedDuration / m_benchmarkCount);
    Serial1.print(", min: ");
    Serial1.print(m_minDuration);
    Serial1.print(", max: ");
    Serial1.println(m_maxDuration);

    resetBenchmarking();
  }
}

