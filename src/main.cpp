/*
    Arduino firmware for the DB2.x
    Run with: 
        pio run --target upload && pio device monitor
*/
#include "assert.h"
#include "cli.h"
#include <ADS1256.h>
#include <tlc59208.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>

/*
    /////////////////////////////////////////////////
            Setup to Google sheets start
    /////////////////////////////////////////////////
*/

// WiFi SSID and Password
//const char* ssid     = "Biosphere";
//const char* password = "pl4stic-sc4nner";
const char* ssid     = "Markus iPhone";
const char* password = "KomNuMand";

// IFTTT URL resource
const char* resource = "/trigger/PlasticScanned/with/key/bl96IM25tg14213NBlSzwH";

// Maker Webhooks IFTTT
const char* server = "maker.ifttt.com";

/*
    /////////////////////////////////////////////////
            Setup to Google sheets end
    /////////////////////////////////////////////////
*/


float clockMHZ = 8; // crystal frequency used on ADS1256
float vRef = 2.5; // voltage reference
ADS1256 adc(clockMHZ,vRef,false); // RESETPIN is permanently tied to 3.3v
//ADS1256 adc(CLKSPEED_MHZ, VREF, false); // this line is cilling the script
TLC59208 ledctrl;
Cli cli;

void SerialClient() {
    cli.handle();
}

void read_adc(int argc, char *argv[])
{
    adc.waitDRDY(); 
    float val = adc.readCurrentChannel(); 
    Serial.println(val , 5);
}



void scan(int argc, char *argv[])
{
    float preScan = adc.readCurrentChannel();   // making a scan without LED's

    float readings[8] = {0};
    for (int i=0; i<8; i++) {                   // taking 8 scan values
        ledctrl.on(i);                          // turning LED "i" on
        delay(5);                               // wait for it to be stable
        adc.waitDRDY();                         // Check if sensor is ready 
        readings[i] = adc.readCurrentChannel(); // Read sensor and save in place "i"
        ledctrl.off(i);                         // turn off LED "i"
    }


    float postScan = adc.readCurrentChannel();  // making a scan without LED's
    
    for (int i=0; i<8; i++) {       
        Serial.print(readings[i], 5);           // this function just prints all the readings
        Serial.print('\t');                     // adds space (tap) between all values
    }
    Serial.println();

    ////////////////////////////////////////
    //     Transmission to google sheets
    ////////////////////////////////////////
    Serial.print("Connecting to "); 
    Serial.print(server);
    
    WiFiClient client;
    int retries = 5;
    while(!!!client.connect(server, 80) && (retries-- > 0)) {
        Serial.print(".");
    }
    Serial.println();
    if(!!!client.connected()) {
        Serial.println("Failed to connect...");
    }
    // All readings to one string
    int multiplier = 10000000 ;
    String ResultsString = String(readings[0]*multiplier) + ";" + 
            String(readings[1]*multiplier) + ";" + 
            String(readings[2]*multiplier) + ";" +
            String(readings[3]*multiplier) + ";" +
            String(readings[4]*multiplier) + ";" +
            String(readings[5]*multiplier) + ";" +
            String(readings[6]*multiplier) + ";" +
            String(readings[7]*multiplier) + ";" +
            String(preScan*multiplier)     + ";" +
            String(postScan*multiplier)
            ; 

    // string as Json object
    String jsonObject = String("{\"value1\":\"") + (ResultsString) +  "\"}";

    client.println(String("POST ") + resource + " HTTP/1.1");
    client.println(String("Host: ") + server); 
    client.println("Connection: close\r\nContent-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonObject.length());
    client.println();
    client.println(jsonObject);
            
    int timeout = 5 * 10; // 5 seconds             
    while(!!!client.available() && (timeout-- > 0)){
        delay(100);
    }
    if(!!!client.available()) {
        Serial.println("No response...");
    }
    while(client.available()){
        Serial.write(client.read());
    }
    
    Serial.println("\nclosing connection");
    client.stop();
    
    SerialClient();
    ////////////////////////////////////////
    //     Transmission to google sheets
    ////////////////////////////////////////
}



void led(int argc, char *argv[])
{
    int num;        // Parameter 1: led number [0..7]
    bool state;     // Parameter 2: led state [on/off]
    if (argc != 3) {
        Serial.println("Usage: led <number> <on/off>");
        return;
    }

    // Parameter checking
    bool args_ok = true;
    num = (int)strtol(argv[1], NULL, 10);
    if (num < 0 || num > 7) args_ok = false;
    if      (strcmp(argv[2], "on") == 0) state = true;
    else if (strcmp(argv[2], "off") == 0) state = false;
    else args_ok = false;

    if (args_ok == false) {
        Serial.println("Usage: Usage: led <number> <on/off>");
    } else {
        state == true ? ledctrl.on(num) : ledctrl.off(num);
    }
}



void help(int argc, char *argv[])
{
    cli.list_commands();
}


// Establish a Wi-Fi connection with your router
void initWifi() {
  Serial.print("Connecting to: "); 
  Serial.print(ssid);
  WiFi.begin(ssid, password);  

  int timeout = 10 * 4; // 10 seconds
  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");

  if(WiFi.status() != WL_CONNECTED) {
     Serial.println("Failed to connect, going back to sleep");
  }

  Serial.print("WiFi connected in: "); 
  Serial.print(millis());
  Serial.print(", IP address: "); 
  Serial.println(WiFi.localIP());
}


void setup()
{

    Serial.begin(9600);
    Serial.println("Serial Up");
    SPI.begin();
    SPI.beginTransaction(
        SPISettings(clockMHZ * 1000000 / 4, MSBFIRST, SPI_MODE1));

    Serial.println("SPI Up");
    Wire.begin();
    Serial.println("Wire Up");
    ledctrl.begin();
    Serial.println("LED Up");
    adc.begin(ADS1256_DRATE_30000SPS,ADS1256_GAIN_1,false); 
    Serial.println("ADC Up");
    adc.setChannel(0,1);    // differential ADC reading 

    initWifi();

    cli.add_command({"scan", scan, "Perform a scan sequence: for each led measure adc value"});
    cli.add_command({"adc", read_adc, "Reads ADC measurement"});
    cli.add_command({"led", led, "Turns an LED <number> on/off <state>.\n\t\t\t\tUsage: led <number> <state>"});
    cli.add_command({"help", help, "Lists all available commands"});
    cli.begin();

    Serial.println("PlasticScanner is initialized!");
}




void loop()
{
    cli.handle();
}

