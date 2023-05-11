const { Client } = require('pg'); //require pg for connecting to postgres
require('dotenv').config(); //require dotenv for getting data from the .env file

//function for connecting to the postgres database and exporting the connected client
module.exports.getClient = async () => {
    const client = new Client({
        
        //should work with the dotenv package but doesnt can try to fix later
        /*
        host: process.env.PG_HOST,
        port: process.env.PG_PORT,
        user: process.env.PG_USER,
        password: process.env.PG_PASSWORD,
        database: process.env.PG_DATABASE,
        */
       
        //Enter database details
        ///*
        host: 'database url',
        port: 'port',
        user: 'username',
        password: 'password',
        database: 'database name',
        //*/
        ssl: true,
      });
      
  await client.connect();
  return client;
};

