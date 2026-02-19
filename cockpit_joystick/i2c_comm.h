#include <Joystick.h>
#include <Wire.h>



// -----------------------------------------------------------------------------------------------
// I2C communication

class I2cCommunication
{
  static bool sendData(int address, int numBytes, uint8_t* buf);
  static void requestData(int address, int numBytes, uint8_t* buf);
  static bool setRegisterAddress(int address, uint8_t regAddress);
  static void sendProcessCommand(int address);
  static bool requestCycle(int address, int numBytes, uint8_t* buf, uint8_t iteration);
};

