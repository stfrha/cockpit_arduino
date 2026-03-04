#include "device_handler.h"


#include "i2c_comm.h"
#include "bit_manipulation.h"


DeviceHandler::DeviceHandler(
  uint8_t deviceId, 
  uint8_t i2cAddr, 
  uint8_t numOfSignalsPerDevice, 
  uint8_t* signalToButtonTable, 
  const uint8_t* joystickButtonUpdates,
  Joystick_* joystick) :
   m_deviceId(deviceId),
   m_i2cAddr(i2cAddr),
   m_numOfSignalsPerDevice(numOfSignalsPerDevice),
   m_signalToButtonTable(signalToButtonTable),
   m_joystickButtonUpdates(joystickButtonUpdates),
   m_joystick(joystick)
{
  m_signalState = 0;
  m_prevSignalState = 0;
}

bool DeviceHandler::reportDeviceExists(void)
{
  uint8_t buf[10];

  if (I2cCommunication::requestCycle(m_i2cAddr, 7, buf, 0))
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
// Initial request to set the previous state variables
{
   uint8_t buf[10];

  if (I2cCommunication::requestCycle(m_i2cAddr, 7, buf, 0))
  {
    m_prevSignalState = m_signalState;
    m_prevAxisState[0] = m_axisState[0];
    m_prevAxisState[1] = m_axisState[1];
  }
}

bool DeviceHandler::getDeviceData(void)
{
   uint8_t buf[10];

  if (I2cCommunication::requestCycle(m_i2cAddr, 11, buf, 0))
  {
    m_signalState =(uint32_t)( ((uint32_t)buf[5] << 24UL) | ((uint32_t)buf[6] << 16UL) | 
      ((uint32_t)buf[7] << 8UL) | (uint32_t)buf[8] );

    uint8_t v = (buf[1] >> 4);
    m_leftRotaryEncoderState += (int8_t)(v | (0 - (v & 0x8)));
    v = (buf[3] >> 4);
    m_rightRotaryEncoderState += (int8_t)(v | (0 - (v & 0x8)));

    m_axisState[0] = ((uint32_t)buf[1] << 8UL) | ((uint32_t)buf[2]);
    m_axisState[1] = ((uint32_t)buf[3] << 8UL) | ((uint32_t)buf[4]);

    return true;

  }

  return false;
}

void DeviceHandler::decodeJoystickButtonChange(void)
{

  // Loop all buttons to see any changes on indivudial buttons, and send joystick command if 
  for (uint8_t i = 0; i < m_numOfSignalsPerDevice; i++)
  {
    // Don't test when the signals has no joystick button defined
    if (m_signalToButtonTable[i] != 255)
    {
      bool bs = BitManipulation::readBit(m_signalState, i);
      if (bs != BitManipulation::readBit(m_prevSignalState, i))
      {

        // Signal state changed
        if (bs)
        {
          m_joystickButtonUpdates[m_signalToButtonTable[i]] = 3;
        }
        else
        {
          m_joystickButtonUpdates[m_signalToButtonTable[i]] = 2;
        }

        m_prevSignalState = BitManipulation::setBit(m_prevSignalState, i, bs);

      }
      else
      {
        // No change, set joystick update to zero
        m_joystickButtonUpdates[m_signalToButtonTable[i]] = 0;

      }
    }
  }
}

void DeviceHandler::evaluateRotaryEncodeChange(void)
{
  // NOTE: For now, this only works for one device having rotary switch. If multiple devices
  // has this, the total steps will add for all devices

  uint8_t firstRotaryButton = m_numOfSignalsPerDevice * 4;

  bool r1 = false;
  bool l1 = false;
  bool r2 = false;
  bool l2 = false;

  if (m_leftRotaryEncoderState > 0)
  {
    // Serial1.print("left counter: leftRotaryEncoderState[device]");
    // Serial1.println(leftRotaryEncoderState[deviceList[deviceIndex]]);
    m_joystick->setButton(firstRotaryButton, true);
    m_leftRotaryEncoderState--;
    l1 = true;
  }

  if (m_leftRotaryEncoderState < 0)
  {
    // Serial1.print("left counter: leftRotaryEncoderState[device]");
    // Serial1.println(leftRotaryEncoderState[deviceList[deviceIndex]]);
    m_joystick->setButton(firstRotaryButton + 1, true);
    m_leftRotaryEncoderState++;
    l2 = true;
  }

  if (m_rightRotaryEncoderState > 0)
  {
    m_joystick->setButton(firstRotaryButton + 2, true);
    m_rightRotaryEncoderState--;
    r1 = true;
  }

  if (m_rightRotaryEncoderState < 0)
  {
    m_joystick->setButton(firstRotaryButton + 3, true);
    m_rightRotaryEncoderState++;
    r2 = true;
  }

  if (r1 || l1 || r2 || l2)
  {
    delay(15);

    if (l1) m_joystick->setButton(firstRotaryButton, false);
    if (l2) m_joystick->setButton(firstRotaryButton + 1, false);
    if (r1) m_joystick->setButton(firstRotaryButton + 2, false);
    if (r2) m_joystick->setButton(firstRotaryButton + 3, false);
  }
}



void DeviceHandler::setAxis(uint8_t axisIndex, int16_t value)
{
  if (m_deviceId == 0)
  {
    if (axisIndex == 0)
    {
      m_joystick->setXAxis(value);
    }
    else if (axisIndex == 1)
    {
      m_joystick->setYAxis(value);
    }
  }
  else if (m_deviceId == 1)
  {
    if (axisIndex == 0)
    {
      m_joystick->setZAxis(value);
    }
    else if (axisIndex == 1)
    {
      m_joystick->setRxAxis(value);
    }
  }
  else if (m_deviceId == 2)
  {
    if (axisIndex == 0)
    {
      m_joystick->setRyAxis(value);
    }
    else if (axisIndex == 1)
    {
      m_joystick->setRzAxis(value);
    }
  }
}

void DeviceHandler::evaluateJoystickAxisChange(void)
{
  for (uint8_t i = 0; i < 2; i++)
  {
    if ((abs(m_axisState[i] - m_prevAxisState[i])) > 5)
    {
      // TODO: Handle axis changed joystick command
      setAxis(i, m_axisState[i]);

      m_prevAxisState[i] = m_axisState[i];
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
    decodeJoystickButtonChange();
    evaluateRotaryEncodeChange();
    evaluateJoystickAxisChange();
  }
}
