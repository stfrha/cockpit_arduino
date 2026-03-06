
#include <cockpit.h>

// Define some terms:
// device signal - one bit in the buffer of one device, corresponds to one button or state of a multi state switch
// joystick button - one button on the usb interface, can be sourced from single or multiple device signals

const int numOfDevices = 3;
const int8_t deviceList[4] = {0, 1, 2, -1};

const int numOfSignalsPerDevice = 30;  // the number of signals in one device, according to the bitvector from the device
const int numOfJoystickButtons = 69;   // The total number of joystick buttons that can be set

uint8_t joystickButtonUpdates[numOfJoystickButtons];

// Device index 0 (device 0)
uint8_t signalToButtonTable[numOfDevices][numOfSignalsPerDevice] = {
  {
    23,
    24,
    25,
    26,
    27,
    28,
    29,
    30,
    31,
    32,
    33,
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
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1
  },
  {
    46,
    47,
    48,
    49,
    50,
    51,
    52,
    53,
    54,
    55,
    56,
    57,
    58,
    59,
    60,
    61,
    62,
    63,
    64,
    65,
    66,
    67,
    68,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1
  },
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
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1
  }
};

uint8_t rotaryEncoderJoystickButtons[4] = { 70, 71, 72, 73}; // Left CW, Left CCW, Right CW, Right CCW

JoystickManager jMgr(
  numOfDevices,
  deviceList, 
  numOfSignalsPerDevice, 
  numOfJoystickButtons, 
  &signalToButtonTable[0][0], 
  joystickButtonUpdates,
  rotaryEncoderJoystickButtons);


uint8_t button39State = 0; // 0 = 39 off, 1 = 39 pending, 2 = 39 On
uint8_t button39PendingCounter = 0;

void handleSpecialJoystickButtonChanges(void)
{
  // This is special handling for combinations of signals to joystick buttons
  // ---of which there are none for the DDI

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
}

bool testMode = false;
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
        for (int i = 0; i < 11; i++)
        {
          Serial1.print(testBuf[i], HEX);
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
    time.sampleTime();
    processDevices();
    time.doPeriodDelay();
  }
}



