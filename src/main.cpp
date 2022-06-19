/*
    Arduino firmware for the DB2.x
    Run with: 
        pio run --target upload && pio device monitor
*/

#include "assert.h"
#include <ADS1256.h>
#include <tlc59208.h>
#include <Arduino.h>
//#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
//#include <WiFiMulti.h>
#include <TFT_eSPI.h> 

#include "model-v1.h" // must come after <arduino.h>


/* /////////////////////////////////////////////////
            BLE setup
*//////////////////////////////////////////////////
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

#define SERVICE_UUID "b5540b68-9717-40c2-8a5f-37d990d3959f"
#define CHARACTERISTIC_UUID "345c11d5-8947-48cf-a27f-46ac37f2657a"



class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



/* /////////////////////////////////////////////////
            Wifi setup
*//////////////////////////////////////////////////
// WiFiMulti wifiMulti;

const int buttonPin = 33;
int buttonState = 0;
float IdentifiedPlasticType = 0;

uint8_t ADCstatus = 0;

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

// // IFTTT URL resource
// const char* resource = "/trigger/PlasticScanned/with/key/bl96IM25tg14213NBlSzwH";

// // Maker Webhooks IFTTT
// const char* server = "maker.ifttt.com";



/* /////////////////////////////////////////////////
            ADC & LED control
*//////////////////////////////////////////////////

float clockMHZ = 8; // crystal frequency used on ADS1256
float vRef = 2.5; // voltage reference
ADS1256 adc(clockMHZ,vRef,false); // RESETPIN is permanently tied to 3.3v
TLC59208 ledctrl;

void initiateBLE() {
    Serial.println("Starting BLE work!");

    BLEDevice::init("PlasticScanner");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                            CHARACTERISTIC_UUID,
                                            BLECharacteristic::PROPERTY_READ |
                                            BLECharacteristic::PROPERTY_WRITE
                                        );

    pCharacteristic->setValue("Ready to save the world!");
    pService->start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("BLE Up!");
}

void NotifyBLE(String data){
    if (deviceConnected) { // notify changed value
        pCharacteristic->setValue((uint8_t*)&data, data.length());
        pCharacteristic->notify();
        value++;
        delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
}

void read_adc(int argc, char *argv[])  // Takes a single sensor reading
{
   // adc.waitDRDY(); 
    float val = adc.readCurrentChannel(); 
    Serial.println(val , 5);
}


void sendData(String data){  // Sends data to google sheets
//     Serial.print("Connecting to "); 
//     Serial.print(server);
    
//     WiFiClient client;
//     int retries = 5;
//     while(!!!client.connect(server, 80) && (retries-- > 0)) {
//         Serial.print(".");
//     }
//     Serial.println();
    
//     if(!!!client.connected()) {
//         Serial.println("Failed to connect...");
//     }

//     if(wifiMulti.run() == WL_CONNECTED) {    
//         client.println(String("POST ") + resource + " HTTP/1.1");
//         client.println(String("Host: ") + server); 
//         client.println("Connection: close\r\nContent-Type: application/json");
//         client.print("Content-Length: ");
//         client.println(data.length());
//         client.println();
//         client.println(data);
                
//         int timeout = 5 * 10; // 5 seconds             
//         while(!!!client.available() && (timeout-- > 0)){
//             delay(100);
//         }
//         if(!!!client.available()) {
//             Serial.println("No response...");
//         }
//         while(client.available()){
//             Serial.write(client.read());
//         }
//         Serial.println("\nclosing connection");
//         client.stop();
//     }
//     else{
//         Serial.println("Not Connected to WiFi");
//         Serial.println("Would have sent the following data:");
//         Serial.println(data);
//     }
Serial.println("Would be sent via wifi");
}

void scan() // Performs a scanning
{
    
    int multiplier = 10000000 ; 
    // Place "scanning" screen here
    //adc.waitDRDY();  
    Serial.println("Data was ready!");

    float preScan = adc.readCurrentChannel()*multiplier;   // making a scan without LED's
    float readings[8] = {0};
    for (int i=0; i<8; i++) {   
                       // taking 8 scan values
        ledctrl.on(i);                          // turning LED "i" on
        Serial.println("LED on ");
        Serial.println(i);
        delay(15);                               // wait for it to be stable
        //adc.waitDRDY();                         // Check if sensor is ready 
        readings[i] = adc.readCurrentChannel(); // Read sensor and save in place "i"
        ledctrl.off(i);                         // turn off LED "i"
        Serial.println("Reading ");
        Serial.println(readings[i]);
        Serial.println();
    }

    //adc.waitDRDY();  
    float postScan = adc.readCurrentChannel()*multiplier;  // making a scan without LED's
    
    for (int i=0; i<8; i++) {       
        Serial.print(readings[i], 5);           // this function just prints all the readings
        Serial.print('\t');                     // adds space (tap) between all values
    }
    Serial.println();

    ////////////////////////////////////////
    //     Transmission to google sheets
    ////////////////////////////////////////

    // All readings to one string
    
    String ResultsString = String(readings[0]) + ";" + 
            String(readings[1]) + ";" + 
            String(readings[2]) + ";" +
            String(readings[3]) + ";" +
            String(readings[4]) + ";" +
            String(readings[5]) + ";" +
            String(readings[6]) + ";" +
            String(readings[7]) + ";" +
            String(preScan)     + ";" +
            String(postScan)
            ; 

    // string as Json object
    String jsonObject = String("{\"value1\":\"") + (ResultsString) +  "\"}";
    //sendData(jsonObject);
    NotifyBLE(jsonObject);

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

void initADC (){
    Serial.println("Starting ADC");
    adc.setChannel(0,1);
    Serial.println("Channel 0,1 set");
    adc.begin(); 
    Serial.println("ADC has begun");

    ADCstatus= adc.getStatus();
    Serial.print("ADC Started: ");
    Serial.println(ADCstatus,BIN);   // print in binary format, ADS1256 should print "110000"



    //adc.setChannel(0,1);
    //adc.begin(ADS1256_DRATE_30000SPS,ADS1256_GAIN_1,false); 
    
    Serial.println("ADC Up");
    //adc.setChannel(0,1);    // differential ADC reading 

    // while (ADCstatus == 0)
    // {
    //     Serial.println("ADC status check");
    //     ADCstatus = adc.getStatus();
    //     Serial.println(ADCstatus,BIN);
        
    //     delay(50);
    // }
    
    // ADCstatus = adc.getStatus();
    // Serial.println(ADCstatus,BIN);
}

void initWifi() { // Establish a Wi-Fi connection with your router
    // wifiMulti.addAP("Markus iPhone", "KomNuMand");
    // wifiMulti.addAP("Biosphere", "pl4stic-sc4nner");
    // wifiMulti.addAP("TSH Guest", "");
    // wifiMulti.addAP("HW45", "defg030ab1");

    // Serial.println("Connecting Wifi...");
    // if(wifiMulti.run() == WL_CONNECTED) {
    //     Serial.println("");
    //     Serial.println("Connected to WiFi:");
    //     Serial.println(WiFi.SSID());
    // }
    // else{
    //     Serial.println("WiFi Failed. Trying again in 30sec");
    // }
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

    initADC();

    initiateBLE();
    //initWifi();
    //Serial.println("Wifi Up");
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
        Serial.println("Triggered!");
        scan();
        LastUseMillis = currentMillis;

    } 
    // Place result screen here


    // // if WiFi is down, try reconnecting
    // if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
    //     Serial.print(millis());
    //     Serial.println("Reconnecting to WiFi...");
    //     WiFi.disconnect();
    //     initWifi();
    //     previousMillis = currentMillis;
    // }
         
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }

    if (currentMillis - LastUseMillis >= SleepTimer) // If no button press for some time -> sleep
    {
        Serial.println("Going to sleep...");
        // Place "Going to sleep screen" + delay here
        esp_deep_sleep_start();
    }
    
   




}

