// created by using resources form https://how2electronics.com/interface-capacitive-soil-moisture-sensor-arduino/#Capacitive_Soil_Moisture_Sensor_Calibration
// and https://wiki.dfrobot.com/Capacitive_Soil_Moisture_Sensor_SKU_SEN0193 '
// use this calibration to find airvalue and watervalue to create a lowest value, highest value.
// the function will use these values to create a % of how wet the ground is.

#include "Arduino.h"
#include <Wire.h>                //I2c communcation library.
#include "ADS1X15.h"             // Analog to digital converter  ads1115 library. ADC is connected to SEN0193 soil moisture sensor.
#define TCAADDR 0x70             // I2c multiplexer address 0x70 in hex, 112 in decimal.
ADS1115 ADS(0x48);               // Analog to digital converter  ads1115, address 0x48 in hex, 72 in decimal.

int32_t sensor_value_SEN0193;   // variable to store sensor values from Analog to digial converter ADS1115. ADC is connected to SEN0193 soil moisture sensor.

// to calibrate, read of the raw values from air and write it down, insert it in air_value
// then put the sensor in water and read of the value from water and write it down

void SEN0193_calibration()
{
  int sensor_raw_value        = 0;
  int sensor_prosentage_value = 0;
  // change the value on air_value and water_value from results from
  const int air_value   = 8700;   //example values from my readings  : 8700
  const int water_value = 192;     //example values from my readings : 192

  sensor_raw_value = ADS.readADC(0); //reads the adc value from pin 0 on the adc
  sensor_prosentage_value = map(sensor_raw_value, air_value, water_value, 0, 100);
  Serial.print("raw data: "); Serial.println(sensor_raw_value);

  if (sensor_prosentage_value >= 100)
  {
    sensor_value_SEN0193 = 100;
  }
  else if (sensor_prosentage_value <= 0)
  {
    sensor_value_SEN0193 = 0;
  }
  else if (sensor_prosentage_value > 0 && sensor_prosentage_value < 100)
  {
    sensor_value_SEN0193 = sensor_prosentage_value;
  }
}
// Functions for i2c multiplexer TCA9548
// Both functions was created by using resources from https://learn.adafruit.com/adafruit-tca9548a-1-to-8-i2c-multiplexer-breakout/arduino-wiring-and-test.

// Function to choose which scl and sda pins to read from on i2c multiplexer TCA9548. where "i" is the choosen scl and sda pins to read from.
void tcaselect(int i) {
  if (i > 7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

// Function to read from selected "t" which sensor connected, returns a decimal address value
int check_sensor_address(int t)
{
  int addr = 0;
  tcaselect(t);
  for (addr; addr <= 127; addr++)
  {
    if (addr == TCAADDR) continue;
    Wire.beginTransmission(addr);
    if (!Wire.endTransmission() && addr != 0)
    {
      return (addr);
    }
  }
  return 0;
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

void check_connector_sensor(int i)
{
  uint8_t sensoradress = check_sensor_address(i);
  tcaselect(i);
  switch (sensoradress)
  {
    case 0x48:   //Analog to digital converter ads1115, connected to SEN0193 soil moisture sensor.
      SEN0193_calibration();
      Serial.print("value_sen; "); Serial.print(sensor_value_SEN0193); Serial.println(" %");
      delay(1000);
      break;
  }
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  Turn_on_off_sensor(true);
}

void loop()
{
  for (int i = 0; i < 6; i++)  //a loop to go thru connector 0 to 5, on the i2c multiplexer.
  { 
    check_connector_sensor(i);
    delay(100);
  }
}
