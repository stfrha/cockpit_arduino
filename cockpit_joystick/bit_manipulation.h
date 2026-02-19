#include <Joystick.h>
#include <Wire.h>



// -----------------------------------------------------------------------------------------------
// Bit manipulation functions

class BitManipulation
{
  static bool readBit(uint32_t vect, uint8_t index);
  static uint32_t setBit(uint32_t vect, uint8_t index, bool x);
};


