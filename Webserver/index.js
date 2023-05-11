//js file for connecting to database
const { getClient } = require('./get-client');

//require express for webhook functionality, body-parser for json parsing and float-array-to-string for converting hex array to float
const express = require("express")
const bodyParser = require("body-parser")
const floatConverter = require("float-array-to-string") //not used but can be used in further itterations

//function to convert hex string to deximal
const hexToDecimal = hex => parseInt(hex, 16);
const binToDecimal = bin => parseInt(bin, 2);

//function to convert signed 32 bit ints to decimal usefull for sensors that have negative values
//pass it a 4byte hex number eks (0x00FF00FF) and it will convert it to a decimal value 
hexToNegPosNum = (hexNum) => {    
    let binString = '';
    
    if(parseInt(hexNum,16).toString(2).length < 32){
        //if the number is less than 32 bits long this padds start of the string with zeros to make it 32 bits long
        //converts it also to string of binary numbers
        binString =  hexToDecimal(hexNum).toString(2).padStart(32,'0')
    }
    else{
        //converts hexadecimal number to binary string
        binString = hexToDecimal(hexNum).toString(2)
    }

    if(binString[0] == '1'){
        // 1 as msb means that number is negative, flipp the bits in the number (~) add 1 and convert it to negative decimal 
        return (-(~(binToDecimal(binString))+1))
    }
    else{
        //0 as msb means the number is positive and we can convert it directly to decimal
        return (binToDecimal(binString))
    }
}

//function for formating data from the JSON file into an usable array that can be inserted into the database
function formatData(data){ 
    //create variables
    let returnArray
    let sensorData = []
    
    sensorData = data.uplink_message.decoded_payload.dataArray //Data from thingsstack in hex format 
    batteryValue = data.uplink_message.decoded_payload.battery //battery value from thingsstack in decimal value
    sensorType =  parseInt(data.uplink_message.decoded_payload.sensor) //Sensor adress in hex format
    sensorboxConnector = parseInt(data.uplink_message.decoded_payload.connector) //Connector number
    sensorboxID = data.end_device_ids.device_id //Device id from thingsstack
    
    switch(sensorType){
        case 23: //Light meter
            sensorData = hexToDecimal(sensorData[0]+sensorData[1]) //takes the hex values from sensordata combines them and turns them into decimal
            returnArray = [sensorType,sensorboxID,sensorboxConnector,sensorData]
            break;
        case 48: //Soil humidity / ADC
            sensorData = hexToDecimal(sensorData[0]+sensorData[1]) //takes the hex values from sensordata combines them and turns them into decimal 
            returnArray = [sensorType,sensorboxID,sensorboxConnector,sensorData]
            break;
        case 39: //Spectrometer
            //Takes two and two hex values from the sensor data array combines them, turns them into decimal and puts them back into sensorData 
            colorArray = []
            for(let i = 0; i < sensorData.length; i+=2){ 
                colorArray.push(hexToDecimal(sensorData[i]+sensorData[i+1]))
            }
            //
            returnArray = [sensorType,sensorboxID,sensorboxConnector,colorArray] 
            break;
        case 61: //SCD30 Sensor
            //diffrent sensor values
            var co2Sensor = hexToDecimal(sensorData[0]+sensorData[1]) //takes the hex values from sensordata [0] and [1] combines them and turns them into decimal
            var tempSensor = hexToNegPosNum(sensorData[2]+sensorData[3]+sensorData[4]+sensorData[5]) //takes hex values at place [2],[3],[4] and [5] from sensorData and decodes them to a negative or positive value
            var humiditySensor = hexToDecimal(sensorData[6]+sensorData[7])  //takes the hex values from sensordata [6] and [7] combines them and turns them into decimal
            
            sensorData = [co2Sensor,tempSensor,humiditySensor] //places co2, temp and humidity data into sensordata
            returnArray = [sensorType,sensorboxID,sensorboxConnector,sensorData] //returns [sensorType, sensorboxID,sensorboxConnector, sensorData[int,pos or neg int,int],]
            break;
        case 60: //Soil temperature
        sensorData = hexToNegPosNum(sensorData[0]+sensorData[1]+sensorData[2]+sensorData[3]) //takes hex values at place from sensorData and decodes them to a negative or positive value
        returnArray = [sensorType,sensorboxID,sensorboxConnector,sensorData]
        break;

        case 255: //battery
        returnArray = [sensorType,sensorboxID,sensorboxConnector,batteryValue]
        break;
        //insert new case her to add another sensor

        default:
            console.log(`Default case...`);
            returnArray = [null,sensorboxID,null,null,null] //returns 0 if there is no data
            break;
    }
    return returnArray
}

async function addData(data){
    //data[0] = sensorType
    //data[1] = sensorboxID
    //data[2] = sensorboxConnector
    //data[3] = sensorData[x] - size depends on amount of data

    const client = await getClient();
    switch(data[0]){
        /* not in use anymore but can be used for somthing else
        case null: //If the first number is null there is nothing connected to the box
            insertRow = await client.query(`INSERT INTO "Skjetleingard_test"(sensor_type,sensorbox_id,sensorbox_connector,value) VALUES(${data[0]},'${data[1]}',${data[2]},${data[3]});`)
            console.log(`Inserted ${insertRow} row`);
            break;
        */
        case 255: //battery //inserts data into the database and prints it to the console
            insertRow = await client.query(`INSERT INTO "Skjetleingard_test"(sensorbox_id,sensorbox_connector,battery) VALUES('${data[1]}',${data[2]},${data[3]});`)
            console.log(`Inserted ${insertRow} row`);
        break;
        case 23: //Soil moisture / ADC //inserts data into the database and prints it to the console
            insertRow = await client.query(`INSERT INTO "Skjetleingard_test"(sensor_type,sensorbox_id,sensorbox_connector,value) VALUES(${data[0]},'${data[1]}',${data[2]},${data[3]});`)
            console.log(`Inserted ${insertRow} row`);
            break;
        case 48: //Light meter //inserts data into the database and prints it to the console
            insertRow = await client.query(`INSERT INTO "Skjetleingard_test"(sensor_type,sensorbox_id,sensorbox_connector,value) VALUES(${data[0]},'${data[1]}',${data[2]},${data[3]});`)
            console.log(`Inserted ${insertRow} row`);
            break;
        case 60: //Soil temperature //inserts data into the database and prints it to the console
            insertRow = await client.query(`INSERT INTO "Skjetleingard_test"(sensor_type,sensorbox_id,sensorbox_connector,value) VALUES(${data[0]},'${data[1]}',${data[2]},${data[3]});`)
            console.log(`Inserted ${insertRow} row`);
        break;
        case 39: //Spectrometer //inserts data into the database and prints it to the console
            insertRow = await client.query(`INSERT INTO "Skjetleingard_test"(sensor_type,sensorbox_id,sensorbox_connector,nm405_425,nm435_455,nm470_490,nm505_525,nm545_565,nm580_600,nm620_640,nm670_690,clear,nir) VALUES(${data[0]},'${data[1]}',${data[2]},${data[3][0]},${data[3][1]},${data[3][2]},${data[3][3]},${data[3][4]},${data[3][5]},${data[3][6]},${data[3][7]},${data[3][8]},${data[3][9]});`)
            console.log(`Inserted ${insertRow} row`);
            break;
        case 61: //SCD30 sensor CO2,Air Temperature, Humidity //inserts data into the database and prints it to the console
            insertRow = await client.query(`INSERT INTO "Skjetleingard_test"(sensor_type,sensorbox_id,sensorbox_connector,co2,temperature,humidity) VALUES(${data[0]}, '${data[1]}', ${data[2]}, ${data[3][0]}, ${data[3][1]}, ${data[3][2]});`)
            console.log(`Inserted ${insertRow} row`);
            break;
        
        
        //insert new case her to add another sensor
        
        default: //if you end up here there is somthing wrong with the formated data or the modified code
            console.log(`Default case... does not insert anything in database`);
            break;
    }

    await client.end();
}

// Initialize express and define a port
const app = express()
//Port to run server on
const PORT = 3000

// Tell express to use body-parser's JSON parsing
app.use(bodyParser.json())  

// Start express on the defined port
app.listen(PORT, () => console.log(`🚀 Server running on port ${PORT}`))

app.use(bodyParser.json())
app.post("/hook", (req, res) => {
    //debugging code, can be seen in the console 
        console.log("----------------")
        console.log("Raw data: ",req.body.uplink_message.decoded_payload.rawInput)
        console.log("Data array: ", req.body.uplink_message.decoded_payload.dataArray)
        console.log("Connector: ",req.body.uplink_message.decoded_payload.connector)
        console.log("Sensor: 0x"+req.body.uplink_message.decoded_payload.sensor, " Decimal: ",req.body.uplink_message.decoded_payload.sensor) 
        console.log("Formated data: ", formatData(req.body))
    //run the addData function with the formatData command on the JSON file sent by thingsstack 
    addData(formatData(req.body))
    console.log("----------------")
    
    res.status(200).end() // Responding is important
})
//...