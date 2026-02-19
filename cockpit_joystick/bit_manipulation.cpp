#include "cockpit_joystick_common.h"


// ------------------------------------------------------------------------
// Bit manipulation functions

static bool BitManipulation::readBit(uint32_t vect, uint8_t index)
{
  return (vect >> index) & 1UL;
}

static uint32_t BitManipulation::setBit(uint32_t vect, uint8_t index, bool x)
{
  return (vect & ~(1UL << index)) | ((uint32_t)x << index);
}

