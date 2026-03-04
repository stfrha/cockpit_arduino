#include <Arduino.h>

#ifndef BitManipulation_h
#define BitManipulation_h



// -----------------------------------------------------------------------------------------------
// Bit manipulation functions

class BitManipulation
{
public:
  static bool readBit(uint32_t vect, uint8_t index);
  static uint32_t setBit(uint32_t vect, uint8_t index, bool x);
};


#endif
