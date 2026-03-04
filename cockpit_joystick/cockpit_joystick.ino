#include <Joystick.h>
#include <Wire.h>


// Define some terms:
// device signal - one bit in the buffer of one device, corresponds to one button or state of a multi state switch
// joystick button - one button on the usb interface, can be sourced from single or multiple device signals

const int numOfDevices = 3;
int8_t deviceList[4] = {0, 1, 3, -1};

const int numOfSignalsPerDevice = 30;  // the number of signals in one device, according to the bitvector from the device
const int numOfJoystickButtons = 55;   // The total number of joystick buttons that can be set

// Device index 0 (device 0)
uint8_t signalToButtonTable[numOfDevices][numOfSignalsPerDevice] = {
  {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    -1,
    -1,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    -1,
    -1
  },
  {
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    26,
    27,
    -1,
    28,
    29,
    30,
    31,
    32,
    33,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1
  },
  {
    34,
    35,
    -1,
    -1,
    -1,
    36,
    37,
    38,
    -1,
    -1,
    40,
    41,
    43,
    44,
    -1,
    45,
    46,
    47,
    48,
    49,
    50,
    51,
    52,
    53,
    54,
    -1,
    -1,
    -1,
    -1,
    -1
  }
};


// Create the Joystick
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_JOYSTICK, 122, 0,
  true, true,   // Device  0 use x-axis and y-axis
  true, true,   // Device  1 use z-axis and Rx-axis
  true, true,   // Device  2 use Ry-axis and Rz-axis
  true, true,   // Device  3 use rudder-axis and trottle-axis
  false, false, false);

uint8_t buf[10];


// -----------------------------------------------------------------------------------------------
// Time manageing functions

unsigned long bmStartTime = 0;
unsigned long cyclePeriod = 16666; // 60 Hz period

// Benchmarking
unsigned long accumulatedDuration = 0;
unsigned long maxDuration = 0;
unsigned long minDuration = 0;
unsigned long benchmarkCount = 0;

void sampleTime(void)
{
  bmStartTime = micros();
  // Serial1.print("Start time: ");
  // Serial1.print(bmStartTime);
}


unsigned long findCycleDuration(void)
{
  unsigned long now = micros();

  // Serial1.print(", start time in findCycleDuration: ");
  // Serial1.print(bmStartTime);
  // Serial1.print(", time now: ");
  // Serial1.print(now);

  if (now < bmStartTime)
  {
    // We have a wrap, unwrap it
    return 4294967295 - bmStartTime + now;
  }

  // Serial1.print(", duration: ");
  // Serial1.println(now - bmStartTime);


  return now - bmStartTime;
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
    Serial1.print("Benchmark processing duration report! avg: ");
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

bool readBit(uint64_t vect, uint8_t index)
{
  return (vect >> index) & 1UL;
}

uint32_t setBit(uint32_t vect, uint8_t index, bool x)
{
  return (vect & ~(1UL << index)) | ((uint32_t)x << index);
}

uint32_t setBit(uint64_t vect, uint8_t index, bool x)
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
int8_t leftRotaryEncoderState[4] = {0, 0, 0, 0};
int8_t rightRotaryEncoderState[4] = {0, 0, 0, 0};
uint32_t prevButtonState[4] = { 0, 0, 0, 0};
int16_t axisState[4][2] = {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}};
int16_t prevAxisState[4][2] = {{ 0, 0}, { 0, 0}, { 0, 0}, { 0, 0}};
uint8_t joystickButtonUpdates[numOfJoystickButtons];  // 0 = no change, 2 = change to release ed, 3 = change to pressed

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

void initiatePreviousData(uint8_t deviceIndex)
{
  // Initial request to set the previous state variables
  if (requestCycle(i2cAddr[deviceList[deviceIndex]], 7, buf, 0))
  {
    prevButtonState[deviceIndex] = buttonState[deviceIndex];
    prevAxisState[deviceIndex][0] = axisState[deviceIndex][0];
    prevAxisState[deviceIndex][1] = axisState[deviceIndex][1];
  }
}

bool getDeviceData(uint8_t deviceIndex)
{
  if (requestCycle(i2cAddr[deviceList[deviceIndex]], 11, buf, 0))
  {
    // Serial1.print("Device: ");
    // Serial1.print(device);
    // Serial1.print(" responded to data request:");

    buttonState[deviceIndex] =(uint32_t)( ((uint32_t)buf[5] << 24UL) | ((uint32_t)buf[6] << 16UL) | ((uint32_t)buf[7] << 8UL) | (uint32_t)buf[8] );
    uint8_t v = (buf[1] >> 4);
    leftRotaryEncoderState[deviceIndex] += (int8_t)(v | (0 - (v & 0x8)));
    v = (buf[3] >> 4);
    rightRotaryEncoderState[deviceIndex] += (int8_t)(v | (0 - (v & 0x8)));
    axisState[deviceIndex][0] = ((uint32_t)buf[1] << 8UL) | ((uint32_t)buf[2]);
    axisState[deviceIndex][1] = ((uint32_t)buf[3] << 8UL) | ((uint32_t)buf[4]);

    // Serial1.print(" - 0x");
    // Serial1.print(buf[5], HEX);
    // Serial1.print(" - 0x");
    // Serial1.print(buf[6], HEX);
    // Serial1.print(" - 0x");
    // Serial1.print(buf[7], HEX);
    // Serial1.print(" - 0x");
    // Serial1.print(buf[8], HEX);

    // Serial1.print("Start time after getDeviceData: ");
    // Serial1.print(bmStartTime);
    // Serial1.print(" for deviceIndex: ");
    // Serial1.println(deviceIndex);

    
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

void decodeJoystickButtonChange(uint8_t deviceIndex)
{

  // Loop all buttons to see any changes on indivudial buttons, and send joystick command if 
  for (uint8_t i = 0; i < numOfSignalsPerDevice; i++)
  {
    // Don't test when the signals has no joystick button defined
    if (signalToButtonTable[deviceIndex][i] != 255)
    {

      // Serial1.print("Start time before decodeJoystickButtonChange: ");
      // Serial1.print(bmStartTime);
      // Serial1.print(" for deviceIndex: ");
      // Serial1.print(deviceIndex);
      // Serial1.print(" at i: ");
      // Serial1.println(i);

      bool bs = readBit(buttonState[deviceIndex], i);
      if (bs != readBit(prevButtonState[deviceIndex], i))
      {

        // Signal state changed
        if (bs)
        {
          // Serial1.print("About to update joystickButtonUpdates at index: ");
          // Serial1.print(signalToButtonTable[deviceIndex][i]);
          // Serial1.print(" where deviceIndex is: ");
          // Serial1.print(deviceIndex);
          // Serial1.print(" and i is: ");
          // Serial1.println(i);

          joystickButtonUpdates[signalToButtonTable[deviceIndex][i]] = 3;
        }
        else
        {
          // Serial1.print("About to update joystickButtonUpdates at index: ");
          // Serial1.print(signalToButtonTable[deviceIndex][i]);
          // Serial1.print(" where deviceIndex is: ");
          // Serial1.print(deviceIndex);
          // Serial1.print(" and i is: ");
          // Serial1.println(i);

          joystickButtonUpdates[signalToButtonTable[deviceIndex][i]] = 2;
        }

        prevButtonState[deviceIndex] = setBit(prevButtonState[deviceIndex], i, bs);

        // Serial1.println("Button state changed");

        // Serial1.print("Device: ");
        // Serial1.print(device);
        // Serial1.println(" responded with data:");
        // Serial1.println(buttonState[device], HEX);
        // Serial1.println("The same with as integer:");
        // Serial1.println(buttonState[device]);

        //Joystick.setButton(i, bs);
        // Joystick.setButton(i + numOfButtonsPerDevice * device, bs);

        // Serial1.print("Before change, the prevButtonState is: ");
        // Serial1.println(prevButtonState[device], HEX);


        // Serial1.print("After change, the prevButtonState is: ");
        // Serial1.println(prevButtonState[device], HEX);
      }
      else
      {
        // No change, set joystick update to zero
        // Serial1.print("About to update joystickButtonUpdates at index: ");
        // Serial1.print(signalToButtonTable[deviceIndex][i]);
        // Serial1.print(" where deviceIndex is: ");
        // Serial1.print(deviceIndex);
        // Serial1.print(" and i is: ");
        // Serial1.println(i);
        joystickButtonUpdates[signalToButtonTable[deviceIndex][i]] = 0;

      }
    }

  }

  // Serial1.print("Start time after decodeJoystickButtonChange: ");
  // Serial1.print(bmStartTime);
  // Serial1.print(" for deviceIndex: ");
  // Serial1.println(deviceIndex);

}

void handleSpecialJoystickButtonChanges(void)
{
  // This is special handling for combinations of signals to joystick buttons

  // RWR rotary switch:
  if ((joystickButtonUpdates[36] != 0) || (joystickButtonUpdates[37] != 0) || (joystickButtonUpdates[38] != 0))
  {
    // Evaluate if button 39 is set or released
    if ((joystickButtonUpdates[36] == 2) && (joystickButtonUpdates[37] == 2) && (joystickButtonUpdates[38] == 2))
    {
      joystickButtonUpdates[39] = 3;
    }
    else if (((joystickButtonUpdates[36] == 3) && (joystickButtonUpdates[37] == 0) && (joystickButtonUpdates[38] == 0)) ||
      ((joystickButtonUpdates[36] == 0) && (joystickButtonUpdates[37] == 3) && (joystickButtonUpdates[38] == 0)) ||
      ((joystickButtonUpdates[36] == 0) && (joystickButtonUpdates[37] == 0) && (joystickButtonUpdates[38] == 3)))
      {
        joystickButtonUpdates[39] = 2;
      }
  }

}

void sendJoystickButtons(void)
{
  // Loop all bits of the joystickButtonUpdate vector to see if any joystick button should be changed.
  for (uint8_t i = 0; i < numOfJoystickButtons; i++)
  {
    if (joystickButtonUpdates[i] == 3)
    {
      Serial1.print("Joystick button: ");
      Serial1.print(i);
      Serial1.println(" is pressed.");
    }
    else if (joystickButtonUpdates[i] == 2)
    {
      Serial1.print("Joystick button: ");
      Serial1.print(i);
      Serial1.println(" is released.");
    }

    // Reset state
    joystickButtonUpdates[i] = 0;
  }

  // Serial1.print(", start time after sendJoystickButtons: ");
  // Serial1.print(bmStartTime);

}

void evaluateRotaryEncodeChange(uint8_t deviceIndex)
{
  // NOTE: For now, this only works for one device having rotary switch. If multiple devices
  // has this, the total steps will add for all devices

  uint8_t numOfButtonsPerDevice = 30;

  uint8_t firstRotaryButton = numOfButtonsPerDevice * 4;

  // firstRotaryButton = 0;
  

  bool r1 = false;
  bool l1 = false;
  bool r2 = false;
  bool l2 = false;

  if (leftRotaryEncoderState[deviceList[deviceIndex]] > 0)
  {
    // Serial1.print("left counter: leftRotaryEncoderState[device]");
    // Serial1.println(leftRotaryEncoderState[deviceList[deviceIndex]]);
    Joystick.setButton(firstRotaryButton, true);
    leftRotaryEncoderState[deviceIndex]--;
    l1 = true;
  }

  if (leftRotaryEncoderState[deviceList[deviceIndex]] < 0)
  {
    // Serial1.print("left counter: leftRotaryEncoderState[device]");
    // Serial1.println(leftRotaryEncoderState[deviceList[deviceIndex]]);
    Joystick.setButton(firstRotaryButton + 1, true);
    leftRotaryEncoderState[deviceIndex]++;
    l2 = true;
  }

  if (rightRotaryEncoderState[deviceList[deviceIndex]] > 0)
  {
    Joystick.setButton(firstRotaryButton + 2, true);
    rightRotaryEncoderState[deviceIndex]--;
    r1 = true;
  }

  if (rightRotaryEncoderState[deviceList[deviceIndex]] < 0)
  {
    Joystick.setButton(firstRotaryButton + 3, true);
    rightRotaryEncoderState[deviceIndex]++;
    r2 = true;
  }

  if (r1 || l1 || r2 || l2)
  {
    delay(15);

    if (l1) Joystick.setButton(firstRotaryButton, false);
    if (l2) Joystick.setButton(firstRotaryButton + 1, false);
    if (r1) Joystick.setButton(firstRotaryButton + 2, false);
    if (r2) Joystick.setButton(firstRotaryButton + 3, false);
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
  else if (device == 3)
  {
    if (axisIndex == 0)
    {
      Joystick.setRudder(value);
    }
    else if (axisIndex == 1)
    {
      Joystick.setThrottle(value);
    }
  }
}

void evaluateJoystickAxisChange(uint8_t deviceIndex)
{
  for (uint8_t i = 0; i < 2; i++)
  {
    if ((abs(axisState[deviceIndex][i] - prevAxisState[deviceIndex][i])) > 5)
    {
      // TODO: Handle axis changed joystick command
      setAxis(deviceList[deviceIndex], i,  axisState[deviceIndex][i]);

      prevAxisState[deviceIndex][i] = axisState[deviceIndex][i];
    }
  }
}


// -----------------------------------------------------------------------------------------------
// Functions for processing all devices in series

void initiateAllDevices(void)
{
  for (uint8_t deviceIndex = 0; deviceIndex < numOfDevices; deviceIndex++)
  {
    if (reportDeviceExists(deviceList[deviceIndex]))
    {
      initiatePreviousData(deviceIndex);
    }
  }

  for (uint8_t buttonIndex = 0; buttonIndex < numOfJoystickButtons; buttonIndex++)
  {
    joystickButtonUpdates[buttonIndex] = 0;
  }

}

void processDevices(void)
{
  for (uint8_t deviceIndex = 0; deviceIndex < numOfDevices; deviceIndex++)
  {
    if (getDeviceData(deviceIndex))
    {
      decodeJoystickButtonChange(deviceIndex);
      evaluateRotaryEncodeChange(deviceIndex);
      evaluateJoystickAxisChange(deviceIndex);
    }
  }

  // TODO: Update local button combinations
  handleSpecialJoystickButtonChanges();
  sendJoystickButtons();

}



bool testMode = false;



void runTest()
{
  if (Serial1.available())
  {
    unsigned char d = Serial1.parseInt();
    Serial1.println("Tests are starting...");

    unsigned long tStartTime = 0;
    unsigned long endTime = 0;
    int numOfCycles = 100;

    tStartTime = micros();
    for (int i = 0; i < numOfCycles; i++)
    {
      requestCycle(i2cAddr[i], 7, buf, i);
    }
    endTime = micros();

    Serial1.println("Tests are complete:");
    Serial1.print("It took: ");
    Serial1.print((float)(endTime - tStartTime) / 1000.0f);
    Serial1.print(" ms, that is: ");
    Serial1.print((float)(endTime - tStartTime) / (float)numOfCycles);
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
  Joystick.setRudderRange(0, 1023);
  Joystick.setThrottleRange(0, 1023);

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

//    delay(250);

  }
  else
  {
    sampleTime();
    processDevices();
    doPeriodDelay();
  }
}



