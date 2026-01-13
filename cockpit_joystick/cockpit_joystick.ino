#include <Joystick.h>
#include <Wire.h>


// Create the Joystick
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, 32, 0,
  true, true,   // Device  0 use x-axis and y-axis
  true, true,   // Device  1 use z-axis and Rx-axis
  true, true,   // Device  2 use Ry-axis and Rz-axis
  false, false, false, false, false);

uint8_t buf[10];


// -----------------------------------------------------------------------------------------------
// Time manageing functions

unsigned long startTime = 0;
unsigned long cyclePeriod = 16666; // 60 Hz period

// Benchmarking
unsigned long accumulatedDuration = 0;
unsigned long maxDuration = 0;
unsigned long minDuration = 0;
unsigned long benchmarkCount = 0;

void sampleTime(void)
{
  startTime = micros();
}

unsigned long findCycleDuration(void)
{
  unsigned long now = micros();

  if (now < startTime)
  {
    // We have a wrap, unwrap it
    return 4294967295 - startTime + now;
  }

  return now - startTime;
}

unsigned long getDelay(void)
{
  unsigned long duration = findCycleDuration();

  benchmarkHandleDuration(duration);

  if (duration > cyclePeriod)
  {
    return 0;
  }

  return cyclePeriod - duration;
}

unsigned doPeriodDelay(void)
{
  delayMicroseconds(getDelay());
}

void resetBenchmarking(void)
{
  accumulatedDuration = 0;
  maxDuration = 0;
  minDuration = 4294967295;
  benchmarkCount = 0;
}

// Returns true is benchmark report is to be produced
bool benchmarkHandleDuration(unsigned long duration)
{
  accumulatedDuration += duration;

  if (duration > maxDuration)
  {
    maxDuration = duration;
  }

  if (duration < minDuration)
  {
    minDuration = duration;
  }

  benchmarkCount++;

  if (benchmarkCount > 500)
  {
    // Report benchmark and then reset
    Serial1.print("Benchmark report! avg: ");
    Serial1.print(accumulatedDuration / benchmarkCount);
    Serial1.print(", min: ");
    Serial1.print(minDuration);
    Serial1.print(", max: ");
    Serial1.println(maxDuration);

    resetBenchmarking();
  }

}

// -----------------------------------------------------------------------------------------------
// Bit manipulation functions

bool readBit(uint32_t vect, uint8_t index)
{
  return (vect >> index) & 1UL;
}

uint32_t setBit(uint32_t vect, uint8_t index, bool x)
{
  return (vect & ~(1UL << index)) | ((uint32_t)x << index);
}



// -----------------------------------------------------------------------------------------------
// I2C communication

bool sendData(int address, int numBytes, uint8_t* buf)
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

void requestData(int address, int numBytes, uint8_t* buf)
{
  int received = Wire.requestFrom(address, numBytes);
  for (int i = 0; i < numBytes; i++)
  {
    buf[i] = Wire.read();
  }
}

bool setRegisterAddress(int address, uint8_t regAddress)
{
  uint8_t buf[1];
  buf[0] = regAddress;
  return sendData(address, 1, buf);
}

void sendProcessCommand(int address)
{
  setRegisterAddress(address, 0);
  delayMicroseconds(250);

  uint8_t buf[1];
  buf[0] = 0x10;  // Command to do general processing
  sendData(address, 1, buf);
  delayMicroseconds(250);
}

bool requestCycle(int address, int numBytes, uint8_t* buf, uint8_t iteration)
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


// -----------------------------------------------------------------------------------------------
// Functions for accessing and evaluating one single device

uint8_t i2cAddr[4] = {0xC, 0xD, 0xE, 0xF};
uint32_t buttonState[4] = { 0, 0, 0, 0};
uint32_t prevButtonState[4] = { 0, 0, 0, 0};
int16_t axisState[4][2] = {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}};
int16_t prevAxisState[4][2] = {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}};

bool reportDeviceExists(uint8_t device)
{
  if (requestCycle(i2cAddr[device], 7, buf, 0))
  {
    Serial1.print("Device: ");
    Serial1.print(device);
    Serial1.println(" responded.");
    return true;
  }
  else
  {
    Serial1.print("Device: ");
    Serial1.print(device);
    Serial1.println(" did not respond.");
  }

  return false;
}

void initiatePreviousData(uint8_t device)
{
  // Initial request to set the previous state variables
  if (requestCycle(i2cAddr[device], 7, buf, 0))
  {
    prevButtonState[device] = buttonState[device];
    prevAxisState[device][0] = axisState[device][0];
    prevAxisState[device][1] = axisState[device][1];
  }
}

bool getDeviceData(uint8_t device)
{
  if (requestCycle(i2cAddr[device], 11, buf, 0))
  {
    // Serial1.print("Device: ");
    // Serial1.print(device);
    // Serial1.print(" responded to data request:");

    buttonState[device] =(uint32_t)( ((uint32_t)buf[5] << 24UL) | ((uint32_t)buf[6] << 16UL) | ((uint32_t)buf[7] << 8UL) | (uint32_t)buf[8] );
    axisState[device][0] = ((uint32_t)buf[1] << 8UL) | ((uint32_t)buf[2]);
    axisState[device][1] = ((uint32_t)buf[3] << 8UL) | ((uint32_t)buf[4]);

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

void evaluateJoystickButtonChange(uint8_t device)
{


  // Loop all buttons to see any changes on indivudial buttons, and send joystick command if 
  for (uint8_t i = 0; i < 32; i++)
  {
    bool bs = readBit(buttonState[device], i);
    if (bs != readBit(prevButtonState[device], i))
    {
      // Button state changed
      // Serial1.println("Button state changed");

      // Serial1.print("Device: ");
      // Serial1.print(device);
      // Serial1.println(" responded with data:");
      // Serial1.println(buttonState[device], HEX);
      // Serial1.println("The same with as integer:");
      // Serial1.println(buttonState[device]);

      Joystick.setButton(i, bs);
      //Joystick.setButton(i + 32 * device, bs);

      // Serial1.print("Before change, the prevButtonState is: ");
      // Serial1.println(prevButtonState[device], HEX);

      prevButtonState[device] = setBit(prevButtonState[device], i, bs);

      // Serial1.print("After change, the prevButtonState is: ");
      // Serial1.println(prevButtonState[device], HEX);
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

void initiateAllDevices(void)
{
  for (uint8_t device = 0; device < 4; device++)
  {
    if (reportDeviceExists(device))
    {
      initiatePreviousData(device);
    }
  }
}

void processDevices(void)
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
