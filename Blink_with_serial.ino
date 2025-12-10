/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://docs.arduino.cc/hardware/

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://docs.arduino.cc/built-in-examples/basics/Blink/
*/
#include <Wire.h>

long unsigned int myDelay = 250;
int timeoutErrorCounter = 0;
int otherErrorCounter = 0;
int rxSizeError = 0;
uint8_t buf[10];


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial1.begin(9600);
  Wire.begin();
  Wire.setWireTimeout(3000, true);
}

void sendData(int address, int numBytes, uint8_t* buf)
{
  Wire.beginTransmission(address);
  Wire.write(0); // Register address
  for (int i = 0; i < numBytes; i++)
  {
    Wire.write(buf[i]);
  }
  byte error = Wire.endTransmission();
  if (error)
  {
    if (error == 5)
    {
      timeoutErrorCounter++;
    }
    else
    {
      otherErrorCounter++;
    }
  }
}

void requestData(int address, int numBytes, uint8_t* buf)
{
  int received = Wire.requestFrom(address, numBytes);
  if (received != numBytes)
  {
    rxSizeError++;
  }
  else
  {
    for (int i = 0; i < numBytes; i++)
    {
      buf[i] = Wire.read();
    }
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

    if (timeoutErrorCounter + otherErrorCounter + rxSizeError > 0)
    {
      Serial1.print("We found this many errors: ");
      Serial1.println(timeoutErrorCounter + otherErrorCounter + rxSizeError);

      Serial1.print("- Timeout errors: ");
      Serial1.println(timeoutErrorCounter);
      Serial1.print("- Other transmission errors: ");
      Serial1.println(otherErrorCounter);
      Serial1.print("- Request data size errors: ");
      Serial1.println(rxSizeError);
    }
    else
    {
      Serial1.println("There were no errors!");
    }
  }
}

// the loop function runs over and over again forever
void loop() 
{
  rxSizeError = 0;
  requestCycle(12, 7, buf, 0);
  if (rxSizeError > 0)
  {

  }
  else
  {
    for (int i = 0; i < 7; i++)
    {
      Serial1.print(buf[i]);
      Serial1.print(" - ");
    }
    Serial1.println("");
  }
  delay(14);

}
