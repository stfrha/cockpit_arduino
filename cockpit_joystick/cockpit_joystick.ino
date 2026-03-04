#include "joystick_manager.h"
#include "i2c_comm.h"
#include "time_management.h"

// Define some terms:
// device signal - one bit in the buffer of one device, corresponds to one button or state of a multi state switch
// joystick button - one button on the usb interface, can be sourced from single or multiple device signals

const int numOfDevices = 3;
const int8_t deviceList[4] = {0, 1, 3, -1};

const int numOfSignalsPerDevice = 30;  // the number of signals in one device, according to the bitvector from the device
const int numOfJoystickButtons = 55;   // The total number of joystick buttons that can be set

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

JoystickManager jMgr(
  numOfDevices,
  deviceList, 
  numOfSignalsPerDevice, 
  numOfJoystickButtons, 
  &signalToButtonTable[0][0], 
  joystickButtonUpdates);


uint8_t button39State = 0; // 0 = 39 off, 1 = 39 pending, 2 = 39 On
uint8_t button39PendingCounter = 0;

void handleSpecialJoystickButtonChanges(void)
{
  // This is special handling for combinations of signals to joystick buttons

  // RWR Rotary switch state machine
  if (button39State == 0)
  {
    if ((joystickButtonUpdates[36] == 0) && (joystickButtonUpdates[37] == 0) && (joystickButtonUpdates[38] == 2))
    {
      button39State = 1;
      button39PendingCounter = 0;
    }
  }
  else if (button39State == 1)
  {
    if ((joystickButtonUpdates[36] == 0) && (joystickButtonUpdates[37] == 3) && (joystickButtonUpdates[38] == 0))
    {
      button39State = 0;
    }
    button39PendingCounter++;

    if (button39PendingCounter >= 7)
    {
      button39State = 3;
      joystickButtonUpdates[39] = 3;
    }
  }
  else if (button39State == 3)
  {
    if ((joystickButtonUpdates[36] == 0) && (joystickButtonUpdates[37] == 0) && (joystickButtonUpdates[38] == 3))
    {
      button39State = 0;
      joystickButtonUpdates[39] = 2;
    }
  }

  // CMDS three way switch
    if ((joystickButtonUpdates[40] == 2) || (joystickButtonUpdates[41] == 2))
    {
      joystickButtonUpdates[42] = 3;
    }
    else if ((joystickButtonUpdates[40] == 3) || (joystickButtonUpdates[41] == 3))
    {
      joystickButtonUpdates[42] = 2;
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



