#include "device_handler.h"
#include <Arduino.h>
#include <Joystick.h>

#ifndef JoystickManager_h
#define JoystickManager_h


class JoystickManager
{
private:
  const int c_numOfDevices;
  const uint8_t c_i2cAddr[4] = {0xC, 0xD, 0xE, 0xF};
  const int8_t* c_deviceList;
  const int c_numOfSignalsPerDevice;
  const int c_numOfJoystickButtons;
  uint8_t* m_signalToButtonTable;
  uint8_t* m_joystickButtonUpdates;  // 0 = no change, 2 = change to release ed, 3 = change to pressed

  Joystick_* m_joystick;
   
  DeviceHandler* m_devices[4];

public:
  JoystickManager(
    const int numOfDevices, 
    const uint8_t* deviceList, 
    const int numOfSignalsPerDevice,
    const int numOfJoystickButtons,
    uint8_t* signalToButtonTable,
    uint8_t* joystickButtonUpdates);

  void initiateAllDevices(void);
  void processDevices(void);
  void sendJoystickButtons(void);
};

#endif
