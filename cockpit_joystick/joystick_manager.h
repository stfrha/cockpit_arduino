#include <Joystick.h>
#include <Wire.h>


uint8_t numOfDevices = 4;
uint8_t i2cAddr[numOfDevices] = {0xC, 0xD, 0xE, 0xF};



class JoystickManager
{
  DeviceHandler* m_devices[4];

  JoystickManager();

  void initiateAllDevices(void);
  void processDevices(void);
};


