/*
    Arduino firmware for the DB2.x
    Run with: 
        pio run --target upload && pio device monitor
*/
#include "assert.h"
#include "cli.h"
#include "ADS1256.h"
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

// Replace with your SSID and Password
const char* ssid     = "TSH Guest";
const char* password = "";

// Replace with your unique IFTTT URL resource
const char* resource = "/trigger/PlasticScanned/with/key/bl96IM25tg14213NBlSzwH";

// How your resource variable should look like, but with your own API KEY (that API KEY below is just an example):
//const char* resource = "/trigger/bme280_readings/with/key/nAZjOphL3d-ZO4N3k64-1A7gTlNSrxMJdmqy3";

// Maker Webhooks IFTTT
const char* server = "maker.ifttt.com";

/*
    /////////////////////////////////////////////////
            Setup to Google sheets end
    /////////////////////////////////////////////////
*/
static const int CLKSPEED_MHZ = 8;
static const float VREF = 2.5;

ADS1256 adc(CLKSPEED_MHZ, VREF, false);
TLC59208 ledctrl;
Cli cli;

void scan(int argc, char *argv[])
{
    float readings[8] = {0};
    for (int i=0; i<8; i++) {
        ledctrl.on(i);
        delay(5);
        adc.waitDRDY(); 
        readings[i] = adc.readCurrentChannel();
        ledctrl.off(i);
    }

    for (int i=0; i<8; i++) {
        Serial.print(readings[i], 5);
        Serial.print('\t');
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
            String(readings[7]*multiplier) 
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

}

void read_adc(int argc, char *argv[])
{
    adc.waitDRDY(); 
    float val = adc.readCurrentChannel();
    Serial.println(val , 5);
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
    //delay(500);
    //MarkusScanner();

}

