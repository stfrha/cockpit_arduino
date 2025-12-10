#include <Joystick.h>
#include <Wire.h>


// Create the Joystick
Joystick_ Joystick;

uint8_t buf[10];


void sendData(int address, int numBytes, uint8_t* buf)
{
  Wire.beginTransmission(address);
  Wire.write(0); // Register address
  for (int i = 0; i < numBytes; i++)
  {
    Wire.write(buf[i]);
  }
  byte error = Wire.endTransmission();
}

void requestData(int address, int numBytes, uint8_t* buf)
{
  int received = Wire.requestFrom(address, numBytes);
  for (int i = 0; i < numBytes; i++)
  {
    buf[i] = Wire.read();
  }
}

void setRegisterAddress(int address, uint8_t regAddress)
{
  uint8_t buf[1];
  buf[0] = regAddress;
  sendData(address, 1, buf);
}

void requestCycle(int address, int numBytes, uint8_t* buf, uint8_t iteration)
{
  setRegisterAddress(address, 0);
  delayMicroseconds(250);
  requestData(address, numBytes,  buf);
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
      requestCycle(12, 7, buf, i);
    }
    endTime = micros();

    Serial1.println("Tests are complete:");
    Serial1.print("It took: ");
    Serial1.print((float)(endTime - startTime) / 1000.0f);
    Serial1.print(" ms, that is: ");
    Serial1.print((float)(endTime - startTime) / (float)numOfCycles);
    Serial1.println(" us per cycle");

    // if (timeoutErrorCounter + otherErrorCounter + rxSizeError > 0)
    // {
    //   Serial1.print("We found this many errors: ");
    //   Serial1.println(timeoutErrorCounter + otherErrorCounter + rxSizeError);

    //   Serial1.print("- Timeout errors: ");
    //   Serial1.println(timeoutErrorCounter);
    //   Serial1.print("- Other transmission errors: ");
    //   Serial1.println(otherErrorCounter);
    //   Serial1.print("- Request data size errors: ");
    //   Serial1.println(rxSizeError);
    // }
    // else
    // {
    //   Serial1.println("There were no errors!");
    // }
  }
}


int buttonPrevState = 0; 
uint16_t axisPrevState = 0;

void setup() {

	// Initialize Joystick Library
	Joystick.begin();

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  Serial1.begin(9600);
  
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  // Initial request to set the previous state variables
  requestCycle(12, 7, buf, 0);

  if (!buf[4]) buttonPrevState = 1;
  axisPrevState = (buf[2] << 8) | (buf[3]);

}

int buttonState = 0;
uint16_t axisState = 0;

void loop() 
{
  requestCycle(12, 7, buf, 0);

  // for (int i = 0; i < 7; i++)
  // {
  //   Serial1.print(buf[i]);
  //   Serial1.print(" - ");
  // }
  // Serial1.println("");

  buttonState = 0;
  if (!buf[4]) buttonState = 1;
  axisState = (buf[2] << 8) | (buf[3]);

  Serial1.print("Button press: ");

  if (buttonState)
  {
    Serial1.print("1");
  }
  else
  {
    Serial1.print("0");
  }

  Serial1.print(", ADC: ");
  Serial1.println(axisState);



  if (buttonState != buttonPrevState) 
  {
    Joystick.setButton(0, buttonState);
    buttonPrevState = buttonState;
  }

  // axisState = (buf[1] << 8) | (buf[2]);

  // if (axisState != axisPrevState) 
  // {
  //   // Joystick.setButton(0, axisState);
  //   axisPrevState = axisState;
  // }

  delay(13);
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
