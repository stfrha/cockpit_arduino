
#include <cockpit.h>

// Define some terms:
// device signal - one bit in the buffer of one device, corresponds to one button or state of a multi state switch
// joystick button - one button on the usb interface, can be sourced from single or multiple device signals

const int numOfDevices = 4;
const int8_t deviceList[4] = {0, 1, 2, 3};

const int numOfSignalsPerDevice = 30;  // the number of signals in one device, according to the bitvector from the device
const int numOfJoystickButtons = 55;   // The total number of joystick buttons that can be set

const bool testMode = false;

uint8_t joystickButtonUpdates[numOfJoystickButtons];

// Device index 0 (device 0)
uint8_t signalToButtonTable[numOfDevices][numOfSignalsPerDevice] = {
  {
    0,
    1,
    2,
    4,
    5,
    6,
    11,
    12,
    13,
    14,
    10,
    3,
    7,
    8,
    9,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    -1,
    -1,
    -1,
    -1,
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
    -1,
    24,
    25,
    26,
    28,
    29,
    30,
    31,
    32,
    33
  },
  {
    34,
    35,
    36,
    37,
    38,
    39,
    40,
    41,
    42,
    43,
    44,
    45,
    46,
    47,
    48,
    49,
    50,
    51,
    -1,
    -1,
    52,
    53,
    54,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1
  }
};

int8_t rotaryEncoderJoystickButtons[numOfDevices][4] = {
  { -1, -1, -1, -1},
  { -1, -1, -1, -1},
  { 90, 91, -1, -1}, // Radar Altimeter CW, Radar Altimeter  CCW
  { -1, -1, -1, -1}
}; 

// int8_t rotaryEncoderJoystickButtons[numOfDevices][4] = {
//   { -1, -1, -1, -1},
//   { -1, -1, -1, -1},
//   { -1, -1, -1, -1}, // Radar Altimeter CW, Radar Altimeter  CCW
//   { -1, -1, -1, -1}
// }; 

JoystickManager jMgr(
  numOfDevices,
  deviceList, 
  numOfSignalsPerDevice, 
  numOfJoystickButtons, 
  &signalToButtonTable[0][0], 
  joystickButtonUpdates,
  &rotaryEncoderJoystickButtons[0][0]);

void handleSpecialJoystickButtonChanges(void)
{
  // This is special handling for combinations of signals to joystick buttons
  // not used for this joystick.
}


void processDevices(void)
{
  jMgr.processDevices();

  // TODO: Update local button combinations
  handleSpecialJoystickButtonChanges();
  
  jMgr.sendJoystickButtons();

}


TimeManagement time;

void setup() 
{


  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial1.begin(9600);

  jMgr.initiateAllDevices();

  time.resetBenchmarking();

  if (testMode)
  {
    jMgr.initiateTestMode();
  }
}

uint8_t testBuf[10];
const uint8_t c_i2cAddr[4] = {0xC, 0xD, 0xE, 0xF};

void loop() 
{

  if (testMode)
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      if (I2cCommunication::requestCycle(c_i2cAddr[i], 11, testBuf, 0))
      {
        jMgr.executeTestMode(testBuf);
      }
    }
    // Serial1.println("");
    Serial1.print('\r');
    delay(2);

//    delay(250);

  }
  else
  {
    time.sampleTime();
    processDevices();
    time.doPeriodDelay();
  }
}



