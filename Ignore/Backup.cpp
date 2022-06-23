/*
    Arduino firmware for the DB2.x
    Run with: 
        pio run --target upload && pio device monitor
*/

/*   General Pin allocation
| MOSI      | 23      | MOSI        | Master out, Slave in          | 
| MISO      | 19      | MISO        | Master in, Slave out          | 
| SCK       | 18      | CLK         | Clock syncronisation          |       
| CS        | 5       | CS          | Chip select                   | 
| PDWN      | 14      | PDWN        | Power down                    | 
| Reset     | 27      | Reset       | Reset                         | 
| DRDY      | 26      | DRDY        | Data Ready                    | 
*/


#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>


/* /////////////////////////////////////////////////
            BLE setup
*//////////////////////////////////////////////////
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pPRESCAN = NULL;
BLECharacteristic* pSCAN1 = NULL;
BLECharacteristic* pSCAN2 = NULL;
BLECharacteristic* pSCAN3 = NULL;
BLECharacteristic* pSCAN4 = NULL;
BLECharacteristic* pSCAN5 = NULL;
BLECharacteristic* pSCAN6 = NULL;
BLECharacteristic* pSCAN7 = NULL;
BLECharacteristic* pSCAN8 = NULL;
BLECharacteristic* pSCANPOST = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

#define SERVICE_UUID    "b5540b68-9717-40c2-8a5f-37d990d3959f"
#define SCAN_PRE_UUID   "345c11d5-8947-48cf-a27f-46ac37f2657a"
#define SCAN_1_UUID     "9c647f48-8681-42e5-9721-90f6838cb9b8"
#define SCAN_2_UUID     "6d5fc8f3-5d5b-4fd9-95e6-bf38c1b41fd3"
#define SCAN_3_UUID     "301d789e-1d7e-4075-8d89-91e5fbe34296"
#define SCAN_4_UUID     "23d1a3c8-fbac-482c-8bb4-a6e02029cd49"
#define SCAN_5_UUID     "8f553f30-b86a-44b2-9e65-5b18655e1f45"
#define SCAN_6_UUID     "cc04d0f9-71f9-4480-aabf-96315939c444"
#define SCAN_7_UUID     "33d7c053-f879-48e4-bdb1-a17de4a178d1"
#define SCAN_8_UUID     "2f69391c-c2f5-47cf-b806-7f22b400c0c1"
#define SCAN_POST_UUID  "b6309642-c6c3-4ac3-b9a6-a7a740ccf11f"



class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



/* /////////////////////////////////////////////////
            LED Setup
*//////////////////////////////////////////////////
#include <NeoPixelBus.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    25

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 2

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGBW);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)




/* /////////////////////////////////////////////////
            Screen setup
*//////////////////////////////////////////////////

#include <TFT_eSPI.h> 
#include <String.h>
#include "Logo5.h"
#include "NotoSansBold36.h"
#include "Roboto24pt.h"
#include "Roboto_Condensed_36.h"
#include "Roboto_Condensed_20.h"
#include "Roboto_Condensed_16.h"
#include "Roboto_Condensed_12.h"


// Define library an sprites
TFT_eSPI tft=TFT_eSPI();
TFT_eSprite logo = TFT_eSprite(&tft);
TFT_eSprite circle = TFT_eSprite(&tft);
TFT_eSprite screenText = TFT_eSprite(&tft);
TFT_eSprite scanningLine = TFT_eSprite(&tft);

// Time keeping
unsigned long pM1 = 0;
unsigned long pM2 = 0;

// Define screen center
int centerX = 120;
int centerY = 120;
String serial_string = "";
int screenDisplayed = 0;

//Colours used are stored here
#define C_RESULT      0x77F4      /* 180,  46, 226 */
#define C_SCANNING    0xFF57      /* 249, 232, 190 */
#define gradWait1     0x95F9
#define gradWait2     0x367F

// Text matrix
//char *loading[] = {"Scanning.", "Scanning..", "Scanning..."};

// Horizontal scrolling
int spriteWidth = 100;
int spriteHeight = 50;
int spacing = 20;
float currentX = centerX-(spriteWidth/2);
int dirX = 1;
int xmax = centerX+(spriteWidth/2)-3;
int xmin = centerX-(spriteWidth/2)+1;


/* /////////////////////////////////////////////////
           Buttons 
*//////////////////////////////////////////////////
const int buttonPowerPin = 32;
const int buttonPin = 33;
int buttonState = 0;
float IdentifiedPlasticType = 0;



/* /////////////////////////////////////////////////
            Deep sleep & timekeeping
*//////////////////////////////////////////////////

#define BUTTON_PIN_BITMASK 0x200000000 // 2^33 in hex
RTC_DATA_ATTR int bootCount = 0;
unsigned long LastUseMillis = 0;
unsigned long AgainTimer = 10000;
unsigned long ReadyTimer = AgainTimer * 3;
unsigned long SleepTimer = AgainTimer * 50;
unsigned long LastScreenMillis = 0;
unsigned long ReadyScreenMillis = 0;
unsigned long AgainScreenMillis = 0;
unsigned long currentMillis = millis();
bool LastResultReady = false;

// For timekeeping of Wifi Reconnect
unsigned long previousMillis = 0;
unsigned long interval = 30000;
unsigned long BootingInterval = 3000;





/* /////////////////////////////////////////////////
            Screens
*//////////////////////////////////////////////////

void ScreenFillBlack() 
{
    tft.fillCircle(centerX, centerY, 130, TFT_BLACK);
    screenText.fillScreen(TFT_BLACK);
}

void ScreenFillGreen() 
{
    tft.fillCircle(centerX, centerY, 130, TFT_DARKGREEN);
    screenText.fillScreen(TFT_DARKGREEN);
}

void ScreenFillRed() 
{
    tft.fillCircle(centerX, centerY, 130, TFT_RED);
    screenText.fillScreen(TFT_RED);
}




void CreateSprites() {
    // Create sprites
    Serial.println("Create sprite initiated");
    logo.createSprite(77, 62);
    circle.createSprite(240,240);
    screenText.createSprite(150, 150);
    scanningLine.createSprite(spriteWidth, spriteHeight);
    Serial.println("Create done");
}

void ScreenStart() {
    // Show start screen
    Serial.println("Start Screen initiated");
    ScreenFillBlack();
    screenText.fillSprite(TFT_BLACK);
    screenText.pushSprite(45, 45);
    logo.setSwapBytes(true);
    logo.pushImage(0,0, 77, 62, Logo5); 
    logo.pushSprite(centerX-(78/2), centerY-(62/2));
    logo.deleteSprite();
    Serial.println("Screen start done");
}

void ScreenReady() {
    Serial.println("Ready screen initiated");
    ScreenFillBlack();
    //create line
    scanningLine.setColorDepth(8);
    scanningLine.drawFastVLine(0,0,50, TFT_GREEN);
    scanningLine.drawFastVLine(1,0,50, TFT_GREEN);
    scanningLine.drawFastVLine(2,0,50, TFT_GREEN);

    screenText.setTextSize(2);
    screenText.setFreeFont(&Roboto_Condensed_20);
    screenText.setTextColor(TFT_WHITE);
    screenText.setTextDatum(4);
    screenText.drawString("to", 75, 75);
    screenText.setTextDatum(7);
    screenText.drawString("Press", 75, 75-20);
    screenText.setTextDatum(1);
    screenText.drawString("scan", 75, 75+20);
    screenText.pushSprite(45, 45);
    Serial.println("Ready screen done");
}   

void ScreenScanningLine(){
    scanningLine.pushSprite(centerX-(spriteWidth/2), centerY-spriteHeight);
    scanningLine.scroll(dirX, 0);

    currentX += dirX;
    
    if(currentX == xmin)  dirX = +1;
    if(currentX == xmax)  dirX = -1;
}

void ScreenScanAgain(){
    Serial.println("Scan again initiated");
    screenText.fillSprite(TFT_ORANGE);
    screenText.pushSprite(45, 45);
    tft.setTextDatum(4);
    tft.setTextSize(6);
    tft.drawString(String("PVC"), centerX, 100);
    tft.setTextDatum(4);
    tft.setTextSize(2);
    tft.drawString("Press button", centerX, 135);
    tft.drawString("to scan again", centerX, 155);
    Serial.println("Scan again done");
}

void ScreenScanning(){
    ScreenFillBlack();
    Serial.println("Scanning screen initiated");
    screenText.setTextDatum(4);
    screenText.setFreeFont(&Roboto_Condensed_12);
    screenText.drawString("Scanning", 75, 75+20);
    screenText.pushSprite(45, 45);
    
    unsigned long cM1 = millis();
    Serial.println("Scanning line initiated");
    while((cM1-pM1) <= 2000){
       cM1 = millis();
       unsigned long cM2 = millis();
       if(cM2-pM2 >= 10){
            ScreenScanningLine();
            pM2 = cM2;
        }
    }
    Serial.println("Scanning screen done");
}

void ScreenResult(){

    // Make screen black
    // make background X color
    // put text
    // Push sprite
    ScreenFillGreen();
    screenText.setFreeFont(&Roboto_Condensed_36);
    screenText.setTextColor(TFT_WHITE);
    screenText.setTextDatum(4);
    screenText.drawString("PP", 75, 75);
    screenText.pushSprite(45, 45);



    // Serial.println("Result screen initiated");
    unsigned long cM1 = millis();

    // tft.fillCircle(centerX, centerY, 120, C_RESULT );
    // //tft.fillCircle(centerX, centerY, 105, TFT_BLACK);
    
    // screenText.setTextSize(2);
    
    while(cM1-pM1 <2500){
        cM1 = millis();
    }
    Serial.println("Result screen done");
}

/* /////////////////////////////////////////////////
            End of Screens
*//////////////////////////////////////////////////

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void read_adc(int argc, char *argv[])  // Takes a single sensor reading
{
    Serial.println("read ADC initiated");
    //adc.waitDRDY(); 
    float val = 99;
    //val = adc.readCurrentChannel(); 
    Serial.println(val , 5);
    Serial.println("read ADC done");
}

void NotifyBLE(String pre, String scan1, String scan2, String scan3, String scan4, String scan5, String scan6, String scan7, String scan8, String post)
{
    Serial.println("Trying to Notify via BLE");
    if (deviceConnected) { // notify changed value
        Serial.println("Device connected to me");
        pPRESCAN    ->  setValue((uint8_t*)&pre, pre.length());
        pPRESCAN    ->  notify();
        Serial.println("Pre sent!");
        delay(3);

        pSCAN1    ->  setValue((uint8_t*)&scan1, scan1.length());
        pSCAN1    ->  notify();
        Serial.println("Scan 1 sent");
        delay(3);

        pSCAN2    ->  setValue((uint8_t*)&scan2, scan2.length());
        pSCAN2    ->  notify();
        Serial.println("Scan 2 sent");
        delay(3);

        pSCAN3    ->  setValue((uint8_t*)&scan3, scan3.length());
        pSCAN3    ->  notify();
        Serial.println("Scan 3 sent");
        delay(3);

        pSCAN4    ->  setValue((uint8_t*)&scan4, scan4.length());
        pSCAN4    ->  notify();
        Serial.println("Scan 4 sent");
        delay(3);

        pSCAN5    ->  setValue((uint8_t*)&scan5, scan5.length());
        pSCAN5    ->  notify();
        Serial.println("Scan 5 sent");
        delay(3);

        pSCAN6    ->  setValue((uint8_t*)&scan6, scan6.length());
        pSCAN6    ->  notify();
        Serial.println("Scan 6 sent");
        delay(3);

        pSCAN7    ->  setValue((uint8_t*)&scan7, scan7.length());
        pSCAN7    ->  notify();
        Serial.println("Scan 7 sent");
        delay(3);
        
        pSCAN8    ->  setValue((uint8_t*)&scan8, scan8.length());
        pSCAN8    ->  notify();
        Serial.println("Scan 8 sent");
        delay(3);

        pSCANPOST    ->  setValue((uint8_t*)&post, post.length());
        pSCANPOST    ->  notify();
        Serial.println("Post sent");
        delay(3); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
         Serial.println("Every value notified via BLE");
    }
    else{
         Serial.println("No device connected");
    }
   
}

void scan() // Performs a scanning
{
    Serial.println("Scan initiated");
    ScreenScanning();
    int multiplier = 10000000 ; 
    // Place "scanning" screen here
    float preScan = esp_random();
    //preScan = adc.readCurrentChannel()*multiplier;   // making a scan without LED's
    float readings[8] = {0};
    for (int i=0; i<8; i++) {   
                       // taking 8 scan values
        //ledctrl.on(i);                          // turning LED "i" on
        delay(5);                               // wait for it to be stable
        //adc.waitDRDY();                         // Check if sensor is ready 
        readings[i] = esp_random();
        //readings[i] = adc.readCurrentChannel()*multiplier; // Read sensor and save in place "i"
        //ledctrl.off(i);                         // turn off LED "i"
    }
    float postScan = esp_random();
    //postScan = adc.readCurrentChannel()*multiplier;  // making a scan without LED's
    
    for (int i=0; i<8; i++) {       
        Serial.print(readings[i], 5);           // this function just prints all the readings
        Serial.print('\t');                     // adds space (tap) between all values
    }
    Serial.println();
    NotifyBLE(String(preScan), 
                String(readings[1]),
                String(readings[2]),
                String(readings[3]),
                String(readings[4]),
                String(readings[5]),
                String(readings[6]),
                String(readings[7]),
                String(readings[8]) ,
                String(postScan));

    Serial.print("Predicted class: ");
    IdentifiedPlasticType = random(1,5); 
    Serial.println(IdentifiedPlasticType);
    LastResultReady = true;
    ScreenResult();
    Serial.println("Scan done");
    
}

void led(int argc, char *argv[]) // controlls the LED's
{
    Serial.println("LED initiated");
    int num;        // Parameter 1: led number [0..7]
    bool state;     // Parameter 2: led state [on/off]
    if (argc != 3) {
        Serial.println("Usage: led <number> <on/off>");
        return;
    }

    // Parameter checking
    /*
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
    */
   Serial.println("LED done");
}

void initiateBLE() {
    Serial.println("Initializing BLE");

    BLEDevice::init("PlasticScanner");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pPRESCAN = pService->createCharacteristic(
                                            SCAN_PRE_UUID,
                                            BLECharacteristic::PROPERTY_READ
                                        );
    BLECharacteristic *pSCAN1 = pService->createCharacteristic(
                                            SCAN_1_UUID,
                                            BLECharacteristic::PROPERTY_READ
                                        );
    BLECharacteristic *pSCAN2 = pService->createCharacteristic(
                                            SCAN_2_UUID,
                                            BLECharacteristic::PROPERTY_READ
                                        );
    BLECharacteristic *pSCAN3 = pService->createCharacteristic(
                                            SCAN_3_UUID,
                                            BLECharacteristic::PROPERTY_READ
                                        );
    BLECharacteristic *pSCAN4 = pService->createCharacteristic(
                                            SCAN_4_UUID,
                                            BLECharacteristic::PROPERTY_READ
                                        );
    BLECharacteristic *pSCAN5 = pService->createCharacteristic(
                                            SCAN_5_UUID,
                                            BLECharacteristic::PROPERTY_READ
                                        );
    BLECharacteristic *pSCAN6 = pService->createCharacteristic(
                                            SCAN_6_UUID,
                                            BLECharacteristic::PROPERTY_READ
                                        );
    BLECharacteristic *pSCAN7 = pService->createCharacteristic(
                                            SCAN_7_UUID,
                                            BLECharacteristic::PROPERTY_READ
                                        );
    BLECharacteristic *pSCAN8 = pService->createCharacteristic(
                                            SCAN_8_UUID,
                                            BLECharacteristic::PROPERTY_READ
                                        );
    BLECharacteristic *pSCANPOST = pService->createCharacteristic(
                                            SCAN_POST_UUID,
                                            BLECharacteristic::PROPERTY_READ
                                        );
    


    pPRESCAN    ->  setValue("42");
    pSCAN1      ->  setValue("42");
    pSCAN2      ->  setValue("42");
    pSCAN3      ->  setValue("42");
    pSCAN4      ->  setValue("42");
    pSCAN5      ->  setValue("42");
    pSCAN6      ->  setValue("42");
    pSCAN7      ->  setValue("42");
    pSCAN8      ->  setValue("42");
    pSCANPOST   ->  setValue("42");
    pService    ->  start();
    // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("BLE Up!");
}

void initiateTFT()
{
    Serial.println("Initializing screen");
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    screenText.loadFont(NotoSansBold36);
    tft.fillScreen(TFT_BLACK);
    CreateSprites();
    ScreenStart();
    Serial.println("Screen Up");
}

void setup()
{
    Serial.begin(9600);
    print_wakeup_reason();
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0); //1 = High, 0 = Low
    Serial.println("Serial Up");
    SPI.begin();
    Serial.println("SPI Up");
    initiateTFT();
    initiateBLE();
    
    
    pinMode(buttonPin, INPUT);
    pinMode(buttonPowerPin, OUTPUT);
    digitalWrite(buttonPowerPin, HIGH);
    LastUseMillis = millis();
    LastScreenMillis = millis();
    Serial.println("Trigger Up");


    Serial.println("PlasticScanner is initialized!");
    Serial.println("6000ms delay to show booting screen");
    delay(6000);
    ScreenReady();

}

void loop()
{
    buttonState = digitalRead(buttonPin);
    currentMillis = millis();
    pM1 = millis(); 
    pM2 = millis();
    
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

    if (buttonState == LOW) {
        Serial.println("button pushed!"); 
        scan();
        LastUseMillis = currentMillis;
        LastScreenMillis = currentMillis;
        ReadyScreenMillis = currentMillis;
        AgainScreenMillis = currentMillis;
        Serial.println("Button done"); 
    } 
    else if (currentMillis - ReadyScreenMillis >= ReadyTimer) // If no button press for some time -> sleep
    {
        ScreenReady();
        ReadyScreenMillis = currentMillis;
    }
    else if (currentMillis - LastUseMillis >= SleepTimer) // If no button press for some time -> sleep
    {
        Serial.println("Going to sleep...");
        ScreenStart();
        delay(3000);
        // Place "Going to sleep screen" + delay here
        esp_deep_sleep_start();
    }
    
}

