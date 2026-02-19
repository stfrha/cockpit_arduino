#include "cockpit_joystick_common.h"


// ------------------------------------------------------------------------
// I2C Communication

static bool I2cCommunication::sendData(int address, int numBytes, uint8_t* buf)
{
  Wire.beginTransmission(address);
  Wire.write(0); // Register address
  for (int i = 0; i < numBytes; i++)
  {
    Wire.write(buf[i]);
  }
  byte error = Wire.endTransmission();
  
  return (error == 0);
}

static void I2cCommunication::requestData(int address, int numBytes, uint8_t* buf)
{
  int received = Wire.requestFrom(address, numBytes);
  for (int i = 0; i < numBytes; i++)
  {
    buf[i] = Wire.read();
  }
}

static bool I2cCommunication::setRegisterAddress(int address, uint8_t regAddress)
{
  uint8_t buf[1];
  buf[0] = regAddress;
  return sendData(address, 1, buf);
}

static void I2cCommunication::sendProcessCommand(int address)
{
  setRegisterAddress(address, 0);
  delayMicroseconds(250);

  uint8_t buf[1];
  buf[0] = 0x10;  // Command to do general processing
  sendData(address, 1, buf);
  delayMicroseconds(250);
}

static bool I2cCommunication::requestCycle(int address, int numBytes, uint8_t* buf, uint8_t iteration)
{
  if (!setRegisterAddress(address, 0))
  {
    // If error, we do not attempt to do more in the cycle for this device
    return false;
  }
  delayMicroseconds(250);
  requestData(address, numBytes,  buf);
  delayMicroseconds(250);
  sendProcessCommand(address);
  return true;
}


