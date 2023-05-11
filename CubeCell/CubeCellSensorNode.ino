/*
 * This program is created for CubeCell - Dev-Board(V2) HTCC-AB01(V2)
 * Created by Ivar Hansgård and Marius B Marthinsen.
 * The purpose of the program is to collect data from diffrent sensors and send it to thingstack thru LoRaWAN. 
 * The program can be modified to include more sensors or to change out sensors, see guide --> 
 */

#include "Arduino.h"
#include "LoRaWan_APP.h" //Lorawan, communication library.
#include <Wire.h>        //I2c communcation library.

// include
#include <BH1750.h>            // Lightsensor bh1750 library.
#include <Adafruit_AS7341.h>   // Multispecter sensor AS7341 library. 
#include "ADS1X15.h"           // Analog to digital converter  ads1115 library. ADC is connected to SEN0193 soil moisture sensor.
#include "SCD30.h"             // Co2, temperature and humidity sensor SCD30 library.
#include "Adafruit_MCP9600.h"  // mcp9600 termocouple i2c amplifier.

// i2c addresses
#define TCAADDR 0x70    // I2c multiplexer address 0x70 in hex, 112 in decimal.
BH1750 lightMeter;      // using library, standard address is 0x23 in hex, 35 in decimal.
Adafruit_AS7341 as7341; // Using library, standard address is 0x39 in hex, 57 in   decimal.
ADS1115 ADS(0x48);      // Analog to digital converter  ads1115, address 0x48 in hex, 72 in decimal.
#define MCP9600 (0x60)  // mcp9600, using library, standard address is 0x60 in hex, 96 in decimal
Adafruit_MCP9600 mcp;
//the address for scd30, Co2, temperature and humidity sensor is set at 0x61 in hex, 97 in decimal


// Array to place data in a specified order before sending.
// Element [0] is port number, element [1} is sensor address. The rest of the elements is sensor data.

int32_t data_array_0[12];
int32_t data_array_1[12];
int32_t data_array_2[12];
int32_t data_array_3[12];
int32_t data_array_4[12];
int32_t data_array_5[12];
int32_t data_array_6[12];
int32_t data_array_7[12];



// declaring sensor arrays and variables for use:
//Arrays
int32_t multispecter_sensor_value[10];  // Data element order,wavelength nm:415,445,480,515,555,590,630,680,clear,nir. Multispecter sensor sensor AS7341.
float   scd30_sensor_value[3];          // Data element order: co2 in ppm, temperature in celsius, humidity in percentage. Co2, temperature and humidity sensor SCD30.
//variables
int32_t sensor_value_bh1750;            // Variable to store sensor values from lightsensor BH1750.
int32_t sensor_value_SEN0193;           // variable to store sensor values from Analog to digial converter ADS1115. ADC is connected to SEN0193 soil moisture sensor.
int32_t sensor_value_mcp9600;           // variable to store snesor values from mcp9600 connected to a thermocouple J.


uint8_t packageNum = 0;



//LoraWAN (find these in The Things Stack when creating a new node)
/* OTAA para*/
uint8_t devEui[] = {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x05, 0xD6, 0x46};
uint8_t appEui[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
uint8_t appKey[] = {0x64, 0x5D, 0xF7, 0x4E, 0x9A, 0x4D, 0x73, 0x37, 0xC5, 0xD4, 0xC2, 0xC5, 0xC6, 0x0D, 0x53, 0x34};

/* ABP para*/
uint8_t nwkSKey[] = { 0x4E, 0xF8, 0x98, 0x1A, 0xFA, 0x6D, 0x20, 0xDD, 0x4F, 0x0F, 0x74, 0x61, 0x97, 0x99, 0x77, 0x9C };
uint8_t appSKey[] = { 0xC0, 0xE0, 0xF6, 0x43, 0xDB, 0xDE, 0x4F, 0xC4, 0xDB, 0x54, 0x7A, 0x42, 0x23, 0x9E, 0x89, 0xBB };
uint32_t devAddr =  ( uint32_t )0x260BBFEB;

/*LoraWan channelsmask, default channels 0-7*/
uint16_t userChannelsMask[6] = { 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 };

/*LoraWan region, select in arduino IDE tools*/
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;

/*LoraWan Class, Class A and Class C are supported*/
DeviceClass_t  loraWanClass = LORAWAN_CLASS;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 15000;

/*OTAA or ABP*/
bool overTheAirActivation = LORAWAN_NETMODE;

/*ADR enable*/
bool loraWanAdr = LORAWAN_ADR;

/* set LORAWAN_Net_Reserve ON, the node could save the network info to flash, when node reset not need to join again */
bool keepNet = LORAWAN_NET_RESERVE;

/* Indicates if the node is sending confirmed or unconfirmed messages */
bool isTxConfirmed = LORAWAN_UPLINKMODE;

/* Application port */
uint8_t appPort = 2;

uint8_t confirmedNbTrials = 4;

//function for preparing sending of data
static void prepareTxFrame( uint8_t port, uint8_t sending_data)
{
  //variables
  uint8_t sensor;
  int32_t data[22];
  uint16_t batteryVoltage = getBatteryVoltage();

  Serial.print(sending_data);

  //check wich data_array to send from with the sending_data variable and fill the data[] array with values from that array
  switch (sending_data) {
    //send data from connector 1 / multiplexer pin 1
    case 0:
      for (uint8_t i = 0; i < 12; i++)
      {
        data[i] = data_array_0[i];
      }
      break;
    //send data from connector 2 / multiplexer pin 2
    case 1:
      for (uint8_t i = 0; i < 12; i++)
      {
        data[i] = data_array_1[i];
      }
      break;
    //send data from connector 3 / multiplexer pin 3
    case 2:
      for (uint8_t i = 0; i < 12; i++)
      {
        data[i] = data_array_2[i];
      }
      break;
    //send data from connector 4 / multiplexer pin 4
    case 3:
      for (uint8_t i = 0; i < 12; i++)
      {
        data[i] = data_array_3[i];
      }
      break;
    //send data from lightsensor / multiplexer pin 5
    case 4:
      for (uint8_t i = 0; i < 12; i++)
      {
        data[i] = data_array_4[i];
      }
      break;
    //send data from the spectrometer / multiplexer pin 6
    case 5:
      for (uint8_t i = 0; i < 12; i++)
      {
        data[i] = data_array_5[i];
      }
      break;

    case 6: //send battery
      data[1] = 0xff;
      break;

    //cases for two extra connectors
    /*
      case 7: //sensor name / multiplexer 7
      for (uint8_t i = 0; i < 12; i++)
      {
        data[i] = data_array_6[i];
      }
      break;

      case 8: //sensor name / multiplexer pin 8
      for (uint8_t i = 0; i < 12; i++)
      {
        data[i] = data_array_7[i];
      }
      break;
    */

    //should not enter this case somthing is wrong if you do
    default:
      Serial.println("Entered default case sending_data");
      break;
  }

  Serial.print(":" );
  Serial.println(data[0]);

  //the first value in the array is the sensor adress, this switch checks which sensor values are present and formats the appdata correctly for sending
  switch (data[1]) {
    case 0x39:  //multispecter sensor

      appDataSize = 23;
      //
      appData[0] = data[0]; //connector
      appData[1] = data[1]; //sensor
      //
      appData[3] = (uint8_t)(data[2] >> 8); //405_425nm
      appData[4] = (uint8_t)(data[2]);
      //
      appData[5] = (uint8_t)(data[3] >> 8); //435_455nm
      appData[6] = (uint8_t)(data[3]);
      //
      appData[7] = (uint8_t)(data[4] >> 8); //nm470_490nm
      appData[8] = (uint8_t)(data[4]);
      //
      appData[9] = (uint8_t)(data[5] >> 8); //505_525nm
      appData[10] = (uint8_t)(data[5]);
      //
      appData[11] = (uint8_t)(data[6] >> 8); //545_565nm
      appData[12] = (uint8_t)(data[6]);
      //
      appData[13] = (uint8_t)(data[7] >> 8); //580_600nm
      appData[14] = (uint8_t)(data[7]);
      //
      appData[15] = (uint8_t)(data[8] >> 8); //620_640nm
      appData[16] = (uint8_t)(data[8]);
      //
      appData[17] = (uint8_t)(data[9] >> 8); //670_690nm
      appData[18] = (uint8_t)(data[9]);
      //
      appData[19] = (uint8_t)(data[10] >> 8); //Clear
      appData[20] = (uint8_t)(data[10]);
      //
      appData[21] = (uint8_t)(data[11] >> 8); //NIR
      appData[22] = (uint8_t)(data[11]);
      //
      break;

    case 0x61:  //CO2 - Temp - Humidity sensor
      appDataSize = 10;
      //
      appData[0] = data[0]; //connector
      appData[1] = data[1]; //sensor
      //
      appData[2] = (uint8_t)(data[2] >> 8); //CO2 first byte
      appData[3] = (uint8_t)(data[2]);  //CO2 second byte
      //
      appData[4] = (uint8_t)(data[3] >> 24); //4 bytes of temperature
      appData[5] = (uint8_t)(data[3]) >> 16; //We need to send 4 bytes of the temp value because this value can be negative,
      appData[6] = (uint8_t)(data[3] >> 8); //it will be converted in the webserver to either negative or posititve
      appData[7] = (uint8_t)(data[3]); // see the hexToPosOrNegNum function
      //
      appData[8] = (uint8_t)(data[4] >> 8); //humidity first byte
      appData[9] = (uint8_t)(data[4]); //humidity second byte

      break;

    case 0x48:  //soil moisture sensor
      appDataSize = 4;
      //
      appData[0] = data[0]; //connector
      appData[1] = data[1]; //sensor
      //
      appData[2] = (uint8_t)(data[2] >> 8); //soil moisture first byte
      appData[3] = (uint8_t)(data[2]);     //soil moisture second byte
      break;

    case 0x23:  //light sensor
      appDataSize = 4;
      //
      appData[0] = data[0]; //connector
      appData[1] = data[1]; //sensor
      //
      appData[2] = (uint8_t)(data[2] >> 8); //lux value first byte
      appData[3] = (uint8_t)(data[2]); //lux value second byte

      break;


    case 0x60:  //mcp9600
      appDataSize = 6;
      //
      appData[0] = data[0]; //connector
      appData[1] = data[1]; //sensor
      //
      appData[2] = (uint8_t)(data[2]  >> 24); //4 bytes of temperature
      appData[3] = (uint8_t)(data[2]) >> 16;  //We need to send 4 bytes of the temp value because this value can be negative,
      appData[4] = (uint8_t)(data[2]  >> 8);  //it will be converted in the webserver to either negative or posititve
      appData[5] = (uint8_t)(data[2]);        //see the hexToPosOrNegNum function.
      break;

    case 0xff: //battery
      appDataSize = 6;
      //
      appData[0] = 9; //connector
      appData[1] = data[1]; //sensor
      //
      appData[2] = (uint8_t)(batteryVoltage >> 8); //battery voltage first byte
      appData[3] = (uint8_t)(batteryVoltage); //battery voltage second byte
      //
      appData[4] = 0; //send 0 because no data
      appData[5] = 0; //send 0 because no data
      break;

    //add more cases here if you add more sensors

    default:
      Serial.println("Entered default case sensor sending code ");
      appDataSize = 4;
      //
      appData[0] = data[0]; //connector
      appData[1] = data[1]; //sensor
      //
      appData[2] = 0; //send 0 because no data
      appData[3] = 0; //send 0 because no data
      break;

  }
}

// ----------------------------

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

// ----------------------------

// Functions to collecting data from sensors.

// Function for lightsensor bh1750, reads the sensor and returns a lux value
// Created by using resources from https://randomnerdtutorials.com/arduino-bh1750-ambient-light-sensor/

void lightsensor_BH1750()
{
  lightMeter.begin();
  for (int k = 0; k < 2; k++)
  {
    delay(500);
    uint16_t lux = lightMeter.readLightLevel();
    delay(100);
    lux = lightMeter.readLightLevel();
    sensor_value_bh1750 = lux;
  }
}


// Function to collect data from multispecter sensor AS7341
// Function uses the function from as7341 library and stores the values inn the array multispecter_sensor_value.
// The function can be adjusted, the adjustments will change the amount of time it uses to read the light and how much gain the output will have.
// Created by using the resources from https://learn.adafruit.com/adafruit-as7341-10-channel-light-color-sensor-breakout/arduino.

void multispecter_as7341()
{
  if (as7341.begin())
  {
    as7341.setATIME(100); // integrator time
    as7341.setASTEP(999); // integrator time
    as7341.setGain(AS7341_GAIN_128X);   //gain, x0.5, x1,x2, x4, x8, x18, x32, x64, x128, x256, x512

    multispecter_sensor_value[0] = as7341.getChannel(AS7341_CHANNEL_415nm_F1); //F1(405-425nm
    multispecter_sensor_value[1] = as7341.getChannel(AS7341_CHANNEL_445nm_F2); //F2(435-455nm
    multispecter_sensor_value[2] = as7341.getChannel(AS7341_CHANNEL_480nm_F3); //F3(470-490nm
    multispecter_sensor_value[3] = as7341.getChannel(AS7341_CHANNEL_515nm_F4); //F4(505-525nm
    multispecter_sensor_value[4] = as7341.getChannel(AS7341_CHANNEL_555nm_F5); //F5(545-565nm
    multispecter_sensor_value[5] = as7341.getChannel(AS7341_CHANNEL_590nm_F6); //F6(580-600nm
    multispecter_sensor_value[6] = as7341.getChannel(AS7341_CHANNEL_630nm_F7); //F7(620-640nm
    multispecter_sensor_value[7] = as7341.getChannel(AS7341_CHANNEL_680nm_F8); //F8(670-690nm
    multispecter_sensor_value[8] = as7341.getChannel(AS7341_CHANNEL_CLEAR);    // Clear
    multispecter_sensor_value[9] = as7341.getChannel(AS7341_CHANNEL_NIR);     // NIR
  }
  if (!as7341.readAllChannels())
  {
    Serial.println("Error reading all channels!");
  }
}

// function to fill the variable sensor_value_sen0193, uses the 0 pin on ADC, values is a % from 0 to 100.
// function is created by using resources from https://wiki.dfrobot.com/Capacitive_Soil_Moisture_Sensor_SKU_SEN0193

void SEN0193()
{ for (int k = 0; k < 2; k++)
  {
    int sensor_raw_value        = 0;
    int sensor_prosentage_value = 0;
    // change the value on air_value and water_value from results from calibration
    const int air_value   = 8700;    //example values from my readings  : 8700
    const int water_value = 192;     //example values from my readings : 192

    sensor_raw_value = ADS.readADC(0); //reads the adc value from pin 0 on the adc
    sensor_prosentage_value = map(sensor_raw_value, air_value, water_value, 0, 100);

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
}


// function to fill variable sensor_value_mcp9600 with temperature from thermocouple sensor type J. values can be negative and positive.
// function is created by using resources from https://learn.adafruit.com/adafruit-mcp9600-i2c-thermocouple-amplifier/arduino and example code
void mcp9600()
{
  mcp.begin(MCP9600);
  delay(500);
  if (! mcp.begin(MCP9600)) {
    Serial.println("Sensor not found. Check wiring!");

  }
  else {
    mcp.setADCresolution(MCP9600_ADCRESOLUTION_18);
    mcp.setThermocoupleType(MCP9600_TYPE_J);
    mcp.setFilterCoefficient(3);
    mcp.enable(true);
    for (int k = 0; k < 2; k++)
    {
      sensor_value_mcp9600 = round(mcp.readThermocouple());
    }
  }
}


// Function to turn on or off the gpio1, by choosing "x" to be either "0"(off) or "1"(on).
// Created by using resources from https://docs.heltec.org/en/node/cubecell/frequently_asked_questions.html?fbclid=IwAR3SiM3gACF898N_-fueX9G3EJ4GC8tko5Bf0S78UlmE5DZAP1OEK2IdbHo
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



// i is connector in use, sensor_array is the array you want to fill.
// the function goes thru connector to see what sensor connected and then fills the chosen array with those values

void fill_data_array(int i, int32_t sensor_array[12])
{
  uint8_t sensoradress = check_sensor_address(i);

  tcaselect(i);

  sensor_array[0] = i;
  sensor_array[1] = sensoradress;
  switch (sensoradress)
  {
    case 0x23:   //lightsensor BH1750.
      lightsensor_BH1750();
      sensor_array[2] = sensor_value_bh1750;

      break;

    case 0x48:   //Analog to digital converter ads1115, connected to SEN0193 soil moisture sensor.
      SEN0193();
      sensor_array[2] = sensor_value_SEN0193;
      break;

    case 0x61:   //Co2, temperature and humidity sensor SCD30.

      scd30.initialize();
      delay(500);
      
      if (scd30.isAvailable())
      {

        scd30.getCarbonDioxideConcentration(scd30_sensor_value); //fills the array with new values.

        uint16_t(scd30_sensor_value);

      }
      for (int i = 0; i < 3; i++)
      {
        sensor_array[i + 2] = scd30_sensor_value[i];
      }
      break;

    case 0x39:   //multispecter sensor AS7341.
      for (int i = 0; i < 2; i++) // has to go thru a loop,needs time to collect data, brute force solution
      {

        multispecter_as7341(); //fills the array with new values.

        for (int i = 0; i < 11; i++)
        {
          sensor_array[i + 2]  = multispecter_sensor_value[i];
        }
      }
      break;

    case 0x60:   //mcp9600.
      mcp9600();
      sensor_array[2] = sensor_value_mcp9600;
      break;
  }

}
void get_sensor_data()  //lage funksjon, av det nedenfor.
{
  // Data collecting part of code.
  // Starts of by turning on sensors and resetting all data arrays to 0.
  // Then reads and stores sensor address values to each individual connector variable called sensoradress.
  // The for loop goes thru each connector and is using the sensor address to sort out which sensor is in use in that port.
  // Then it uses the sensor address to choose which function to use on that connector thereafter it fills the variable or array with values.
  // the array is formated as data_array_ [connector, sensor, data values, ...]

  Turn_on_off_sensor(true); //turns on or off gpio0, true = on, false = off
  delay(2000); //adjust this according to delay you want

  //reset data_array_ to zero
  memset(data_array_0, 0, sizeof(data_array_0)); //etter array er brukt sett til 0
  memset(data_array_1, 0, sizeof(data_array_1));
  memset(data_array_2, 0, sizeof(data_array_2));
  memset(data_array_3, 0, sizeof(data_array_3));
  memset(data_array_4, 0, sizeof(data_array_4));
  memset(data_array_5, 0, sizeof(data_array_5));
  memset(data_array_6, 0, sizeof(data_array_6));
  memset(data_array_7, 0, sizeof(data_array_7));

  for (int i = 0; i < 8; i++)  //a loop to go thru connector 0 to 7, on the i2c multiplexer.
  {
    switch (i)
    {
      case 0: //Connector 0 on i2c multiplexer
        fill_data_array(0, data_array_0);
        break;

      case 1: //connector 1
        fill_data_array(1, data_array_1);
        break;

      case 2: //connector 2
        fill_data_array(2, data_array_2);
        break;

      case 3: //connector 3
        fill_data_array(3, data_array_3);
        break;

      case 4: //connector 4
        fill_data_array(4, data_array_4);
        break;

      case 5: //connector 5
        fill_data_array(5, data_array_5);

        break;

      case 6: //connector 6
        fill_data_array(6, data_array_6);
        break;

      case 7: //connector 7
        fill_data_array(7, data_array_7);
        break;
    }
  }

  delay(2000);
  Turn_on_off_sensor(false);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup start");  //to mark start for testing.
  Wire.begin();


#if(AT_SUPPORT)
  enableAt();
#endif
  deviceState = DEVICE_STATE_INIT;
  LoRaWAN.ifskipjoin();
}


void loop()
{
  switch ( deviceState )
  {
    case DEVICE_STATE_INIT:
      {
#if(AT_SUPPORT)
        getDevParam();
#endif
        printDevParam();
        LoRaWAN.init(loraWanClass, loraWanRegion);
        deviceState = DEVICE_STATE_JOIN;
        break;
      }
    case DEVICE_STATE_JOIN:
      {
        LoRaWAN.join();
        break;
      }
    case DEVICE_STATE_SEND:
      {
        if (packageNum == 0 )
        {
          get_sensor_data();
        }

        delay(500);
        prepareTxFrame(appPort, packageNum);
        LoRaWAN.send();
        packageNum++;
        if (packageNum == 7) //change this to 9 if you want to add two more connectors
        {
          packageNum = 0;
          //put to sleep here -- deepsleep/ sleep ? --> watchdog ?
        }
        deviceState = DEVICE_STATE_CYCLE;
        break;
      }

    case DEVICE_STATE_CYCLE:
      {
        // Schedule next packet transmission
        txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
        LoRaWAN.cycle(txDutyCycleTime);
        deviceState = DEVICE_STATE_SLEEP;
        break;
      }
    case DEVICE_STATE_SLEEP:
      {
        LoRaWAN.sleep();
        break;
      }
    default:
      {
        deviceState = DEVICE_STATE_INIT;
        break;
      }
  }




}
