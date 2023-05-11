//Main function for converting data from CubeCell
function decodeUplink(input) {
  //create an array
  let byteArr = [];
  let batteryValue = 0;
  //puts all the bytes sent from the cubecell as a string form into byteArr
  for(let i = 0; i < input.bytes.length; i++){
    byteArr.push(input.bytes[i].toString(16));
  }
  
  //puts first value in byteArr into connector and deletes it
  let connector = byteArr.shift() 
  //puts first value in byteArr into sensor and deletes it
  let sensor = byteArr.shift()

  //if connector = 9 then we are sending the battery value of the box
  if(connector == 9){
    //puts first value in battery_byte1 into connector and deletes it
    let battery_byte1 = byteArr.shift() 
    //puts first value in battery_byte2 into connector and deletes it
    let battery_byte2 = byteArr.shift() 
    //Takes battery_byte 1 and 2 combines them and turns the string into decimal
    batteryValue = parseInt((battery_byte1+battery_byte2),16) 
    sensor = 255
  }
  
  //return a JSON file with all the variables
  return {
    data: {
      rawInput: input, //Raw input bytes (only used for debugging in the webserver)
      battery: batteryValue, //Battery value as a decimal number
      connector: connector, //The connector number in hex
      sensor: sensor, //The sensor adress in hex
      dataArray: byteArr //The rest of byteArr which should be the sensor values
    },
    warnings: [], // optional
    errors: [] // optional (if set, the decoding failed)
  };
}