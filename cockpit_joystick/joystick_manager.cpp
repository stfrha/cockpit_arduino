#include "joystick_manager.h"

#include "i2c_comm.h"



JoystickManager::JoystickManager(
  const int numOfDevices, 
  const uint8_t* deviceList, 
  const int numOfSignalsPerDevice,
  const int numOfJoystickButtons,
  uint8_t* signalToButtonTable,
  uint8_t* joystickButtonUpdates) :
   c_numOfDevices(numOfDevices),
   c_deviceList(deviceList),
   c_numOfSignalsPerDevice(numOfSignalsPerDevice),
   c_numOfJoystickButtons(numOfJoystickButtons),
   m_signalToButtonTable(signalToButtonTable),
   m_joystickButtonUpdates(joystickButtonUpdates)
{
  m_joystick = new Joystick_(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, 
     122, 0,
     true, true,   // Device  0 use x-axis and y-axis
     true, true,   // Device  1 use z-axis and Rx-axis
     true, true,   // Device  2 use Ry-axis and Rz-axis
     true, true,   // Device  3 use rudder-axis and trottle-axis
     false, false, false);
   
   
  for (int deviceIndex = 0; deviceIndex < c_numOfDevices; deviceIndex++)
  {
    m_devices[deviceIndex] = new DeviceHandler(
      c_deviceList[deviceIndex], 
      c_i2cAddr[c_deviceList[deviceIndex]],
      c_numOfSignalsPerDevice,
      &m_signalToButtonTable[deviceIndex * c_numOfSignalsPerDevice],
      m_joystickButtonUpdates,
      m_joystick);
  }


}


void JoystickManager::initiateAllDevices(void)
{
  I2cCommunication::initializeI2c();

  Serial1.println("Initiating devices...");
  for (uint8_t device = 0; device < c_numOfDevices; device++)
  {
    m_devices[device]->initiateDevice();
  }
}

void JoystickManager::processDevices(void)
{
  for (uint8_t device = 0; device < c_numOfDevices; device++)
  {
    m_devices[device]->processDevice();
  }
}

void JoystickManager::sendJoystickButtons(void)
{
  // Loop all bits of the joystickButtonUpdate vector to see if any joystick button should be changed.
  for (uint8_t i = 0; i < c_numOfJoystickButtons; i++)
  {
    if (m_joystickButtonUpdates[i] == 3)
    {
      Serial1.print("Joystick button: ");
      Serial1.print(i);
      Serial1.println(" is pressed.");
    }
    else if (m_joystickButtonUpdates[i] == 2)
    {
      Serial1.print("Joystick button: ");
      Serial1.print(i);
      Serial1.println(" is released.");
    }

    // Reset state
    m_joystickButtonUpdates[i] = 0;
  }

  // Serial1.print(", start time after sendJoystickButtons: ");
  // Serial1.print(bmStartTime);

}

