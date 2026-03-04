#include <Arduino.h>
#include <Joystick.h>



class DeviceHandler
{
private:
  uint8_t m_deviceId;
  uint8_t m_numOfSignalsPerDevice;
  uint8_t m_i2cAddr;
  uint32_t m_signalState = 0;
  int8_t m_leftRotaryEncoderState;
  int8_t m_rightRotaryEncoderState;
  uint32_t m_prevSignalState = 0;
  int16_t m_axisState[2] = { 0, 0};
  int16_t m_prevAxisState[2] = { 0, 0};
  uint8_t* m_signalToButtonTable;
  uint8_t* m_joystickButtonUpdates;
  Joystick_* m_joystick;
  
public:
  DeviceHandler(
    uint8_t deviceId, 
    uint8_t i2cAddr, 
    uint8_t numOfSignalsPerDevice, 
    uint8_t* signalToButtonTable, 
    const uint8_t* joystickButtonUpdates,
    Joystick_* joystick);

  bool reportDeviceExists(void);
  void initiatePreviousData(void);
  bool getDeviceData(void);
  void evaluateDeviceSignalChange(void);
  void decodeJoystickButtonChange(void);
  void evaluateRotaryEncodeChange(void);
  // void evaluateJoystickButtonChange(void);
  void setAxis(uint8_t axisIndex, int16_t value);
  void evaluateJoystickAxisChange(void);
  void initiateDevice(void);
  void processDevice(void);

};

