#include <Joystick.h>
#include <Wire.h>


// Create the Joystick
Joystick_ Joystick;

uint8_t buf[10];


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
      Serial1.println("Button state changed");

      Serial1.print("Device: ");
      Serial1.print(device);
      Serial1.println(" responded with data:");
      Serial1.println(buttonState[device], HEX);
      Serial1.println("The same with as integer:");
      Serial1.println(buttonState[device]);

      Joystick.setButton(i, bs);
      //Joystick.setButton(i + 32 * device, bs);

      Serial1.print("Before change, the prevButtonState is: ");
      Serial1.println(prevButtonState[device], HEX);

      prevButtonState[device] = setBit(prevButtonState[device], i, bs);

      Serial1.print("After change, the prevButtonState is: ");
      Serial1.println(prevButtonState[device], HEX);
    }
  }
}

void evaluateJoystickAxisChange(uint8_t device)
{
  for (int i = 0; i < 2; i++)
  {
    if ((abs(axisState[device][i] - prevAxisState[device][i])) > 5)
    {
      // TODO: Handle axis changed joystick command

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


void setup() {

	// Initialize Joystick Library
	Joystick.begin();

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial1.begin(9600);
  
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  initiateAllDevices();
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
    processDevices();

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
