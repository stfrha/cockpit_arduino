#include <Joystick.h>
#include <Wire.h>




class DeviceHandler
{
  uint8_t m_deviceId;
  uint8_t m_i2cAddr;
  uint32_t m_signalState = 0;
  uint32_t m_prevSignalState = 0;
  int16_t m_axisState[2] = { 0, 0};
  int16_t m_prevAxisState[2] = { 0, 0};

  DeviceHandler(uint8_t deviceId, uint8_t i2cAddr);

  bool reportDeviceExists(void);
  void initiatePreviousData(void);
  bool getDeviceData(void);
  void evaluateDeviceSignalChange(void);
  void evaluateJoystickButtonChange(void);
  void setAxis(uint8_t axisIndex, int16_t value);
  void evaluateJoystickAxisChange();
  void initiateDevice(void);
  void processDevice(void);

};

