#include <Joystick.h>
#include <Wire.h>




// class diagram:
// TimeManagement - handles cycle durations, single instance, non-static
// BitManipulation - static functions for setting and reading bits
// I2cCommunicatoin - handle all comms, scan be static?
// DeviceHandler - handles one PIC–device, non-static, holds all info about the device like address...
// JoystickManager - top level class owning all DeviceHandlers and run main loop. Owns the Joystick interface etc



// Create the Joystick
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, 25*4, 0,
  true, true,   // Device  0 use x-axis and y-axisd:\git_dummy\cockpit_arduino\cockpit_joystick\cockpit_joystick_common.h
  true, true,   // Device  1 use z-axis and Rx-axis
  true, true,   // Device  2 use Ry-axis and Rz-axis
  false, false, false, false, false);

uint8_t buf[10];

uint8_t numOfDevices = 4;
uint8_t i2cAddr[numOfDevices] = {0xC, 0xD, 0xE, 0xF};


// -----------------------------------------------------------------------------------------------
// Time manageing functions

public class TimeManagement
{
  unsigned long m_startTime = 0;
  unsigned long m_cyclePeriod = 16666; // 60 Hz period

  // Benchmarking
  unsigned long m_accumulatedDuration = 0;
  unsigned long m_maxDuration = 0;
  unsigned long m_minDuration = 0;
  unsigned long m_benchmarkCount = 0;

  TimeManagement();

  void sampleTime(void);
  unsigned long findCycleDuration(void);
  unsigned long getDelay(void);
  unsigned doPeriodDelay(void);
  void resetBenchmarking(void);

  // Returns true is benchmark report is to be produced
  bool benchmarkHandleDuration(unsigned long duration);

};




// -----------------------------------------------------------------------------------------------
// Bit manipulation functions

class BitManipulation
{
  static bool readBit(uint32_t vect, uint8_t index);
  static uint32_t setBit(uint32_t vect, uint8_t index, bool x);
};



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

class JoystickManager
{
  DeviceHandler* m_devices[4];

  JoystickManager();

  void initiateAllDevices(void);
  void processDevices(void);
{
  for (uint8_t device = 0; device < 4; device++)
  {
    if (getDeviceData(device))
    {
      evaluateJoystickButtonChange(device);
      evaluateJoystickAxisChange(device);
    }
  }
}


};




// -----------------------------------------------------------------------------------------------
// Functions for accessing and evaluating one single device
uint8_t numOfDevices = 4;
uint8_t i2cAddr[numOfDevices] = {0xC, 0xD, 0xE, 0xF};
uint32_t signalState[numOfDevices] = { 0, 0, 0, 0};
uint32_t prevSignalState[numOfDevices] = { 0, 0, 0, 0};
int16_t axisState[numOfDevices][2] = {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}};
int16_t prevAxisState[numOfDevices][2] = {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}};


void evaluateDeviceSignalChange(uint8_t device)
{

  // Loop all buttons to see any changes on indivudial buttons, and build a 
  for (uint8_t i = 0; i < numOfButtonsPerDevice; i++)
  {
    bool bs = readBit(signalState[device], i);
    if (bs != readBit(prevSignalState[device], i))
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
      Joystick.setButton(i + numOfButtonsPerDevice * device, bs);

      // Serial1.print("Before change, the prevSignalState is: ");
      // Serial1.println(prevSignalState[device], HEX);

      prevSignalState[device] = setBit(prevSignalState[device], i, bs);

      // Serial1.print("After change, the prevSignalState is: ");
      // Serial1.println(prevSignalState[device], HEX);
    }
  }

}


void evaluateJoystickButtonChange(uint8_t device)
{

  uint8_t numOfButtonsPerDevice = 25;

  // Loop all buttons to see any changes on indivudial buttons, and send joystick command if 
  for (uint8_t i = 0; i < numOfButtonsPerDevice; i++)
  {
    bool bs = readBit(signalState[device], i);
    if (bs != readBit(prevSignalState[device], i))
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
      Joystick.setButton(i + numOfButtonsPerDevice * device, bs);

      // Serial1.print("Before change, the prevSignalState is: ");
      // Serial1.println(prevSignalState[device], HEX);

      prevSignalState[device] = setBit(prevSignalState[device], i, bs);

      // Serial1.print("After change, the prevSignalState is: ");
      // Serial1.println(prevSignalState[device], HEX);
    }
  }
}

void setAxis(uint8_t device, uint8_t axisIndex, int16_t value)
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

void evaluateJoystickAxisChange(uint8_t device)
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


// -----------------------------------------------------------------------------------------------
// Functions for processing all devices in series


bool testMode = false;



void runTest()
{
  if (Serial1.available())
  {
    unsigned char d = Serial1.parseInt();
    Serial1.println("Tests are starting...");

    unsigned long startTime = 0;
    unsigned long endTime = 0;
    int numOfCycles = 100;

    startTime = micros();
    for (int i = 0; i < numOfCycles; i++)
    {
      requestCycle(i2cAddr[i], 7, buf, i);
    }
    endTime = micros();

    Serial1.println("Tests are complete:");
    Serial1.print("It took: ");
    Serial1.print((float)(endTime - startTime) / 1000.0f);
    Serial1.print(" ms, that is: ");
    Serial1.print((float)(endTime - startTime) / (float)numOfCycles);
    Serial1.println(" us per cycle");

  }
}


void setup() 
{

  Joystick.setXAxisRange(0, 1023);
  Joystick.setYAxisRange(0, 1023);
  Joystick.setZAxisRange(0, 1023);
  Joystick.setRxAxisRange(0, 1023);
  Joystick.setRyAxisRange(0, 1023);
  Joystick.setRzAxisRange(0, 1023);

	// Initialize Joystick Library
	Joystick.begin();

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial1.begin(9600);
  
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  initiateAllDevices();

  resetBenchmarking();
}


void loop() 
{

  if (testMode)
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      if (requestCycle(i2cAddr[i], 11, buf, 0))
      {
        for (int i = 0; i < 11; i++)
        {
          Serial1.print(buf[i], HEX);
          Serial1.print(" - 0x");
        }
      }
    }
    Serial1.println("");
    delay(16);
  }
  else
  {
    sampleTime();
    processDevices();
    doPeriodDelay();
  }
}




// // the loop function runs over and over again forever
// void loop() 
// {
//   rxSizeError = 0;
//   if (rxSizeError > 0)
//   {

//   }
//   else
//   {
//     for (int i = 0; i < 7; i++)
//     {
//       Serial1.print(buf[i]);
//       Serial1.print(" - ");
//     }
//     Serial1.println("");
//   }

// }
