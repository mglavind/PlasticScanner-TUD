/*
    Arduino firmware for the DB2.x
    Run with: 
        pio run --target upload && pio device monitor
*/

#include "assert.h"
#include <ADS1256.h>
#include <tlc59208.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiMulti.h>
#include "model-v1.h" // must come after <arduino.h>


/* /////////////////////////////////////////////////
            Wifi setup
*//////////////////////////////////////////////////
WiFiMulti wifiMulti;

const int buttonPin = 33;
int buttonState = 0;
float IdentifiedPlasticType = 0;



/* /////////////////////////////////////////////////
            Deep sleep & timekeeping
*//////////////////////////////////////////////////

#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex
RTC_DATA_ATTR int bootCount = 0;
unsigned long LastUseMillis = 0;
unsigned long SleepTimer = 100000;
unsigned long currentMillis = millis();

// For timekeeping of Wifi Reconnect
unsigned long previousMillis = 0;
unsigned long interval = 30000;




/* /////////////////////////////////////////////////
            Machine Learning
*//////////////////////////////////////////////////

// Creating a machine learning based classifier
Eloquent::ML::Port::XGBClassifier classifier;



/* /////////////////////////////////////////////////
            Setup to Google sheets start
*//////////////////////////////////////////////////

// IFTTT URL resource
const char* resource = "/trigger/PlasticScanned/with/key/bl96IM25tg14213NBlSzwH";

// Maker Webhooks IFTTT
const char* server = "maker.ifttt.com";



/* /////////////////////////////////////////////////
            ADC & LED control
*//////////////////////////////////////////////////

float clockMHZ = 8; // crystal frequency used on ADS1256
float vRef = 2.5; // voltage reference
ADS1256 adc(clockMHZ,vRef,false); // RESETPIN is permanently tied to 3.3v
TLC59208 ledctrl;




void read_adc(int argc, char *argv[])  // Takes a single sensor reading
{
    adc.waitDRDY(); 
    float val = adc.readCurrentChannel(); 
    Serial.println(val , 5);
}


void sendData(String data){  // Sends data to google sheets
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

    if(wifiMulti.run() == WL_CONNECTED) {    
        client.println(String("POST ") + resource + " HTTP/1.1");
        client.println(String("Host: ") + server); 
        client.println("Connection: close\r\nContent-Type: application/json");
        client.print("Content-Length: ");
        client.println(data.length());
        client.println();
        client.println(data);
                
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
    else{
        Serial.println("Not Connected to WiFi");
        Serial.println("Would have sent the following data:");
        Serial.println(data);
    }
}

void scan() // Performs a scanning
{
    // Place "scanning" screen here
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
    sendData(jsonObject);

    Serial.print("Predicted class: ");
    IdentifiedPlasticType = classifier.predict(readings);
    Serial.println(IdentifiedPlasticType);
}


    



void led(int argc, char *argv[]) // controlls the LED's
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



void initWifi() { // Establish a Wi-Fi connection with your router
    wifiMulti.addAP("Markus iPhone", "KomNuMand");
    wifiMulti.addAP("Biosphere", "pl4stic-sc4nner");
    wifiMulti.addAP("TSH Guest", "");

    Serial.println("Connecting Wifi...");
    if(wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("Connected to WiFi:");
        Serial.println(WiFi.SSID());
    }
    else{
        Serial.println("WiFi Failed. Trying again in 30sec");
    }
}


void setup()
{
    Serial.begin(9600);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0); //1 = High, 0 = Low
    Serial.println("Serial Up");
    SPI.begin();
    SPI.beginTransaction(
        SPISettings(clockMHZ * 1000000 / 4, MSBFIRST, SPI_MODE1));
    Serial.println("SPI Up");
    // Place Booting screen here
    Wire.begin();
    Serial.println("Wire Up");
    ledctrl.begin();
    Serial.println("LED Up");
    adc.begin(ADS1256_DRATE_30000SPS,ADS1256_GAIN_1,false); 
    Serial.println("ADC Up");
    adc.setChannel(0,1);    // differential ADC reading 
    initWifi();

    pinMode(buttonPin, INPUT);
    LastUseMillis = millis();
    Serial.println("Trigger Up");
    Serial.println("PlasticScanner is initialized!");
}




void loop()
{
    buttonState = digitalRead(buttonPin);
    currentMillis = millis();

    if (buttonState == LOW) {
        scan();
        LastUseMillis = currentMillis;

    } 
    // Place result screen here


    // if WiFi is down, try reconnecting
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
        Serial.print(millis());
        Serial.println("Reconnecting to WiFi...");
        WiFi.disconnect();
        initWifi();
        previousMillis = currentMillis;
    }
    
    if (currentMillis - LastUseMillis >= SleepTimer) // If no button press for some time -> sleep
    {
        Serial.println("Going to sleep...");
        // Place "Going to sleep screen" + delay here
        esp_deep_sleep_start();
    }
    
}

