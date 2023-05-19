//This is a modified version of the original code for calibrating the scd30
// the original can be found https://github.com/Seeed-Studio/Seeed_SCD30/blob/master/examples/SCD30_auto_calibration/SCD30_auto_calibration.ino
// note must be running for 7 days, and spend 1 hour outside every day
// the sensor has to be plugged inn plugg 0


#include "Arduino.h"
#include <Wire.h>                //I2c communcation library.
#include "SCD30.h"
#define TCAADDR 0x70             // I2c multiplexer address 0x70 in hex, 112 in decimal.


#if defined(ARDUINO_ARCH_AVR)
#pragma message("Defined architecture for ARDUINO_ARCH_AVR.")
#define SERIAL Serial
#elif defined(ARDUINO_ARCH_SAM)
#pragma message("Defined architecture for ARDUINO_ARCH_SAM.")
#define SERIAL SerialUSB
#elif defined(ARDUINO_ARCH_SAMD)
#pragma message("Defined architecture for ARDUINO_ARCH_SAMD.")
#define SERIAL SerialUSB
#elif defined(ARDUINO_ARCH_STM32F4)
#pragma message("Defined architecture for ARDUINO_ARCH_STM32F4.")
#define SERIAL SerialUSB
#else
#pragma message("Not found any architecture.")
#define SERIAL Serial
#endif

// Functions for i2c multiplexer TCA9548
// Both functions was created by using resources from https://learn.adafruit.com/adafruit-tca9548a-1-to-8-i2c-multiplexer-breakout/arduino-wiring-and-test.
// Function to choose which scl and sda pins to read from on i2c multiplexer TCA9548. where "i" is the choosen scl and sda pins to read from.
void tcaselect(int i) {
  if (i > 7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}


void Turn_on_off_sensor(int x)
{
  pinMode(GPIO1, OUTPUT);
  if (x == 0)
  {
    digitalWrite(GPIO1, LOW);
  }
  if (x == 1)
  {
    digitalWrite(GPIO1, HIGH);
  }
}

void setup() {
  Turn_on_off_sensor(true);
  delay(1000);
  tcaselect(0);
  Turn_on_off_sensor(true);
  Wire.begin();
  SERIAL.begin(115200);
  SERIAL.println("SCD30 Raw Data");
  scd30.initialize();
  //Calibration for minimum 7 days,after this ,close auto self calibration operation.
  scd30.setAutoSelfCalibration(1);
}

void loop() {
  float result[3] = {0};

  if (scd30.isAvailable()) {
    scd30.getCarbonDioxideConcentration(result);
    SERIAL.print("Carbon Dioxide Concentration is: ");
    SERIAL.print(result[0]);
    SERIAL.println(" ppm");
    SERIAL.println(" ");
    SERIAL.print("Temperature = ");
    SERIAL.print(result[1]);
    SERIAL.println(" ℃");
    SERIAL.println(" ");
    SERIAL.print("Humidity = ");
    SERIAL.print(result[2]);
    SERIAL.println(" %");
    SERIAL.println(" ");
    SERIAL.println(" ");
    SERIAL.println(" ");
  }

  delay(2000);
}
