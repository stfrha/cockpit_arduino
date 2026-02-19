#include "cockpit_joystick_common.h"


// ------------------------------------------------------------------------
// Time functions


TimeManagement::TimeManagement()
{
  m_startTime = 0;
  m_cyclePeriod = 16666; // 60 Hz period
  m_accumulatedDuration = 0;
  m_maxDuration = 0;
  m_minDuration = 0;
  m_benchmarkCount = 0;

  resetBenchmarking();
}


void TimeManagement::sampleTime(void)
{
  m_startTime = micros();
}

unsigned long TimeManagement::findCycleDuration(void)
{
  unsigned long now = micros();

  if (now < m_startTime)
  {
    // We have a wrap, unwrap it
    return 4294967295 - m_startTime + now;
  }

  return now - m_startTime;
}

unsigned long TimeManagement::getDelay(void)
{
  unsigned long duration = findCycleDuration();

  benchmarkHandleDuration(duration);

  if (duration > m_cyclePeriod)
  {
    return 0;
  }

  return m_cyclePeriod - duration;
}

unsigned TimeManagement::doPeriodDelay(void)
{
  delayMicroseconds(getDelay());
}

void TimeManagement::resetBenchmarking(void)
{
  m_accumulatedDuration = 0;
  m_maxDuration = 0;
  m_minDuration = 4294967295;
  m_benchmarkCount = 0;
}

bool TimeManagement::benchmarkHandleDuration(unsigned long duration)
{
  m_accumulatedDuration += duration;

  if (duration > m_maxDuration)
  {
    m_maxDuration = duration;
  }

  if (duration < m_minDuration)
  {
    m_minDuration = duration;
  }

  m_benchmarkCount++;

  if (m_benchmarkCount > 500)
  {
    // Report benchmark and then reset
    Serial1.print("Benchmark duration report! avg: ");
    Serial1.print(m_accumulatedDuration / m_benchmarkCount);
    Serial1.print(", min: ");
    Serial1.print(m_minDuration);
    Serial1.print(", max: ");
    Serial1.println(m_maxDuration);

    resetBenchmarking();
  }
}


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


DeviceHandler::DeviceHandler(uint8_t deviceId, uint8_t i2cAddr)
{
  m_deviceId = deviceId;
  m_i2cAddr = i2cAddr;
  m_signalState = 0;
  m_prevSignalState = 0;
  m_axisState[2] = { 0, 0};
  m_prevAxisState[2] = { 0, 0};
}

bool DeviceHandler::reportDeviceExists(void)
{
  if (requestCycle(m_i2cAddr, 7, buf, 0))
  {
    Serial1.print("Device: ");
    Serial1.print(m_deviceId);
    Serial1.println(" responded.");
    return true;
  }
  else
  {
    Serial1.print("Device: ");
    Serial1.print(m_deviceId);
    Serial1.println(" did not respond.");
  }

  return false;
}

void DeviceHandler::initiatePreviousData(void)
{
  // Initial request to set the previous state variables
  if (requestCycle(m_i2cAddr, 7, buf, 0))
  {
    prevSignalState = signalState;
    prevAxisState[0] = axisState[0];
    prevAxisState[1] = axisState[1];
  }
}

bool DeviceHandler::getDeviceData(void)
{
  if (requestCycle(m_i2cAddr, 11, buf, 0))
  {
    // Serial1.print("Device: ");
    // Serial1.print(device);
    // Serial1.print(" responded to data request:");

    signalState =(uint32_t)( ((uint32_t)buf[5] << 24UL) | ((uint32_t)buf[6] << 16UL) | ((uint32_t)buf[7] << 8UL) | (uint32_t)buf[8] );
    axisState[0] = ((uint32_t)buf[1] << 8UL) | ((uint32_t)buf[2]);
    axisState[1] = ((uint32_t)buf[3] << 8UL) | ((uint32_t)buf[4]);

    // Serial1.print(" - 0x");
    // Serial1.print(buf[5], HEX);
    // Serial1.print(" - 0x");
    // Serial1.print(buf[6], HEX);
    // Serial1.print(" - 0x");
    // Serial1.print(buf[7], HEX);
    // Serial1.print(" - 0x");
    // Serial1.print(buf[8], HEX);

    return true;

    // Serial1.print(", ADC0: ");
    // Serial1.print(axisState);

    // Serial1.print(", ADC1: ");
    // Serial1.print(axisState);

    // Serial1.print("#");
    // Serial1.print(i);
    // Serial1.print(", button states: ");
  }

  return false;
}

void DeviceHandler::evaluateDeviceSignalChange(uint8_t device)
{

  // Loop all buttons to see any changes on indivudial buttons, and build a 
  for (uint8_t i = 0; i < numOfButtonsPerDevice; i++)
  {
    bool bs = readBit(signalState, i);
    if (bs != readBit(prevSignalState, i))
    {
      // Button state changed
      // Serial1.println("Button state changed");

      // Serial1.print("Device: ");
      // Serial1.print(device);
      // Serial1.println(" responded with data:");
      // Serial1.println(signalState[device], HEX);
      // Serial1.println("The same with as integer:");
      // Serial1.println(signalState[device]);

      //Joystick.setButton(i, bs);
      Joystick.setButton(i + numOfButtonsPerDevice * m_deviceId, bs);

      // Serial1.print("Before change, the prevSignalState is: ");
      // Serial1.println(prevSignalState[device], HEX);

      prevSignalState = setBit(prevSignalState, i, bs);

      // Serial1.print("After change, the prevSignalState is: ");
      // Serial1.println(prevSignalState[device], HEX);
    }
  }
}

void DeviceHandler::evaluateJoystickButtonChange(void)
{

  uint8_t numOfButtonsPerDevice = 25;

  // Loop all buttons to see any changes on indivudial buttons, and send joystick command if 
  for (uint8_t i = 0; i < numOfButtonsPerDevice; i++)
  {
    bool bs = readBit(signalState, i);
    if (bs != readBit(prevSignalState, i))
    {
      // Button state changed
      // Serial1.println("Button state changed");

      // Serial1.print("Device: ");
      // Serial1.print(device);
      // Serial1.println(" responded with data:");
      // Serial1.println(signalState[device], HEX);
      // Serial1.println("The same with as integer:");
      // Serial1.println(signalState[device]);

      //Joystick.setButton(i, bs);
      Joystick.setButton(i + numOfButtonsPerDevice * m_deviceId, bs);

      // Serial1.print("Before change, the prevSignalState is: ");
      // Serial1.println(prevSignalState[device], HEX);

      prevSignalState = setBit(prevSignalState, i, bs);

      // Serial1.print("After change, the prevSignalState is: ");
      // Serial1.println(prevSignalState[device], HEX);
    }
  }
}

void DeviceHandler::setAxis(uint8_t device, uint8_t axisIndex, int16_t value)
{
  if (device == 0)
  {
    if (axisIndex == 0)
    {
      Joystick.setXAxis(value);
    }
    else if (axisIndex == 1)
    {
      Joystick.setYAxis(value);
    }
  }
  else if (device == 1)
  {
    if (axisIndex == 0)
    {
      Joystick.setZAxis(value);
    }
    else if (axisIndex == 1)
    {
      Joystick.setRxAxis(value);
    }
  }
  else if (device == 2)
  {
    if (axisIndex == 0)
    {
      Joystick.setRyAxis(value);
    }
    else if (axisIndex == 1)
    {
      Joystick.setRzAxis(value);
    }
  }
}

void DeviceHandler::evaluateJoystickAxisChange(uint8_t device)
{
  for (uint8_t i = 0; i < 2; i++)
  {
    if ((abs(axisState[device][i] - prevAxisState[device][i])) > 5)
    {
      // TODO: Handle axis changed joystick command
      setAxis(device, i,  axisState[device][i]);

      prevAxisState[device][i] = axisState[device][i];
    }
  }
}

void DeviceHandler::initiateDevice(void)
{
  if (reportDeviceExists())
  {
    initiatePreviousData();
  }
}

void DeviceHandler::processDevice(void)
{
  if (getDeviceData())
  {
    evaluateJoystickButtonChange();
    evaluateJoystickAxisChange();
  }
}



JoystickManager::JoystickManager()
{
  for (int device = 0; device < 4; device++)
  {
    m_devices[device] = new DeviceHandler(device, i2cAddr[device]);
  }
}



void JoystickManager::initiateAllDevices(void)
{
  for (uint8_t device = 0; device < 4; device++)
  {
    m_devices[device].initiateDevice();
  }
}

void JoystickManager::processDevices(void)
{
  for (uint8_t device = 0; device < 4; device++)
  {
    m_devices[device].processDevice();
  }
}
