#include <Joystick.h>
#include <Wire.h>


// Create the Joystick
Joystick_ Joystick;

uint8_t buf[10];


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


uint8_t i2cAddr[4] = {0xC, 0xD, 0xE, 0xF};
int buttonPrevState[4] = {0, 0, 0, 0}; 
uint16_t axisPrevState[4] = {0, 0, 0, 0};
int buttonState = 0;
uint16_t axisState = 0;
bool testMode = false;

void reportDeviceExists(uint8_t i)
{
  if (requestCycle(i2cAddr[i], 7, buf, 0))
  {
    Serial1.print("Device: ");
    Serial1.print(i);
    Serial1.println(" responded.");
  }
  else
  {
    Serial1.print("Device: ");
    Serial1.print(i);
    Serial1.println(" did not respond.");
  }
}

void initiatePreviousData(uint8_t i)
{
  // Initial request to set the previous state variables
  if (requestCycle(i2cAddr[i], 7, buf, 0))
  {
    if (!buf[4]) buttonPrevState[i] = 1;
    axisPrevState[i] = (buf[2] << 8) | (buf[3]);
  }
}

void getDeviceData(int i)
{
  if (requestCycle(i2cAddr[i], 7, buf, 0))
  {
    buttonState = 0;
    if (!buf[4]) buttonState = 1;
    axisState = (buf[2] << 8) | (buf[3]);

    Serial1.print("#");
    Serial1.print(i);
    Serial1.print(", button press: ");

    if (buttonState)
    {
      Serial1.print("1");
    }
    else
    {
      Serial1.print("0");
    }

    Serial1.print(", ADC: ");
    Serial1.print(axisState);
    Serial1.print(" - ");

    if (buttonState != buttonPrevState[i]) 
    {
      Joystick.setButton(0, buttonState);
      buttonPrevState[i] = buttonState;
    }
  }
}

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


void setup() {

	// Initialize Joystick Library
	Joystick.begin();

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial1.begin(9600);
  
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  for (uint8_t i = 0; i < 4; i++)
  {
    reportDeviceExists(i);
    initiatePreviousData(i);
  }
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
          Serial1.print(buf[i]);
          Serial1.print(" - ");
        }
      }
    }
    Serial1.println("");
    delay(500);
  }
  else
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      getDeviceData(i);
    }
    Serial1.println("");


    // axisState = (buf[1] << 8) | (buf[2]);

    // if (axisState != axisPrevState) 
    // {
    //   // Joystick.setButton(0, axisState);
    //   axisPrevState = axisState;
    // }

    delay(13);
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
