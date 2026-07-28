
#include <cockpit.h>

// Define some terms:
// device signal - one bit in the buffer of one device, corresponds to one button or state of a multi state switch
// joystick button - one button on the usb interface, can be sourced from single or multiple device signals

const int numOfDevices = 3;
const int8_t deviceList[4] = {0, 1, 3, -1};

const int numOfSignalsPerDevice = 30;  // the number of signals in one device, according to the bitvector from the device
const int numOfJoystickButtons = 72;   // The total number of joystick buttons that can be set

uint8_t joystickButtonUpdates[numOfJoystickButtons];

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
    26,
    27,
    -1,
    -1
  },
  {
    59,
    60,
    61,
    62,
    63,
    64,
    65,
    66,
    -1,
    -1,
    28,
    29,
    -1,
    30,
    31,
    -1,
    32,
    33,
    34,
    35,
    67,
    68,
    69,
    70,
    71,
    -1,
    -1,
    -1,
    -1,
    -1
  },
  {
    36,
    37,
    57,
    58,
    -1,
    38,
    39,
    40,
    -1,
    -1,
    42,
    43,
    45,
    46,
    -1,
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
    -1,
    -1,
    -1,
    -1,
    -1
  }
};

uint8_t rotaryEncoderJoystickButtons[4] = { 90, 91, 92, 93}; // Left CW, Left CCW, Right CW, Right CCW

JoystickManager jMgr(
  numOfDevices,
  deviceList, 
  numOfSignalsPerDevice, 
  numOfJoystickButtons, 
  &signalToButtonTable[0][0], 
  joystickButtonUpdates,
  rotaryEncoderJoystickButtons);


uint8_t button41State = 0; // 0 = 41 off, 1 = 41 pending, 2 = 41 On
uint8_t button41PendingCounter = 0;

void handleSpecialJoystickButtonChanges(void)
{
  // This is special handling for combinations of signals to joystick buttons

  // RWR Rotary switch state machine
  if (button41State == 0)
  {
    if ((joystickButtonUpdates[38] == 0) && (joystickButtonUpdates[39] == 0) && (joystickButtonUpdates[40] == 2))
    {
      button41State = 1;
      button41PendingCounter = 0;
    }
  }
  else if (button41State == 1)
  {
    if ((joystickButtonUpdates[38] == 0) && (joystickButtonUpdates[39] == 3) && (joystickButtonUpdates[40] == 0))
    {
      button41State = 0;
    }
    button41PendingCounter++;

    if (button41PendingCounter >= 7)
    {
      button41State = 3;
      joystickButtonUpdates[41] = 3;
    }
  }
  else if (button41State == 3)
  {
    if ((joystickButtonUpdates[38] == 0) && (joystickButtonUpdates[39] == 0) && (joystickButtonUpdates[40] == 3))
    {
      button41State = 0;
      joystickButtonUpdates[41] = 2;
    }
  }

  // CMDS three way switch
    if ((joystickButtonUpdates[42] == 2) || (joystickButtonUpdates[43] == 2))
    {
      joystickButtonUpdates[44] = 3;
    }
    else if ((joystickButtonUpdates[42] == 3) || (joystickButtonUpdates[43] == 3))
    {
      joystickButtonUpdates[44] = 2;
    }
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



