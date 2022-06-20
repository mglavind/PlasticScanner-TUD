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
#include <TFT_eSPI.h> 
#include <String.h>
#include "Logo5.h"


/* /////////////////////////////////////////////////
            Screen
*//////////////////////////////////////////////////

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
char *loading[] = {"Scanning.", "Scanning..", "Scanning..."};

// Horizontal scrolling
int spriteWidth = 100;
int spriteHeight = 50;
int spacing = 20;
float currentX = centerX-(spriteWidth/2);
int dirX = 1;
int xmax = centerX+(spriteWidth/2)-3;
int xmin = centerX-(spriteWidth/2)+1;


/* /////////////////////////////////////////////////
            Wifi setup
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
    screenText.fillSprite(TFT_BLACK);
    screenText.pushSprite(45, 45);
    tft.fillRectVGradient(0, 0, 240, 240, gradWait1, gradWait2);
    tft.fillCircle(centerX, centerY, 106, TFT_BLACK);

    //create line
    scanningLine.setColorDepth(8);
    scanningLine.drawFastVLine(0,0,50, TFT_GREEN);
    scanningLine.drawFastVLine(1,0,50, TFT_GREEN);
    scanningLine.drawFastVLine(2,0,50, TFT_GREEN);

    screenText.setTextSize(2);
    screenText.setTextColor(TFT_WHITE, TFT_BLACK);
    screenText.setTextDatum(4);
    screenText.drawString("button to", 75, 75);
    screenText.setTextDatum(7);
    screenText.drawString("Press", 75, 75-30);
    screenText.setTextDatum(1);
    screenText.drawString("scan", 75, 75+30);
    screenText.pushSprite(45, 45);
    Serial.println("Ready screen done");
}   

void ScreenScanningLine(){
    Serial.println("Scanning line initiated");
    scanningLine.pushSprite(centerX-(spriteWidth/2), centerY-spriteHeight);
    scanningLine.scroll(dirX, 0);

    currentX += dirX;
    
    if(currentX == xmin)  dirX = +1;
    if(currentX == xmax)  dirX = -1;
    Serial.println("scanning line done");
}

void ScreenScanAgain(){
    Serial.println("Scan again initiated");
    screenText.fillSprite(TFT_BLACK);
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
    Serial.println("Scanning screen initiated");
    screenText.fillSprite(TFT_BLACK);
    screenText.pushSprite(45, 45);
    tft.fillScreen(TFT_BLACK);
    screenText.setTextSize(3);
    screenText.drawString("Scanning", 75, 75+10);
    screenText.pushSprite(45, 45);
    
    unsigned long cM1 = millis();
    
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
    Serial.println("Result screen initiated");
    screenText.fillSprite(TFT_BLACK);
    screenText.pushSprite(45, 45);

    unsigned long cM1 = millis();

    tft.fillCircle(centerX, centerY, 120, C_RESULT );
    tft.fillCircle(centerX, centerY, 105, TFT_BLACK);
    
    tft.setTextDatum(4);
    tft.setTextSize(6);
    tft.drawString(String("PVC"), centerX, 100);
    tft.setTextSize(2);
    tft.drawString("97%", centerX, 135);
    
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

void sendData(String data){  // Sends data to google sheets
    Serial.println("Send data initiated");
    Serial.println("Send data done");
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
            String(readings[7]) + ";"  +
            String(preScan)     + ";" +
            String(postScan)
            ; 

    // string as Json object
    String jsonObject = String("{\"value1\":\"") + (ResultsString) +  "\"}";
    // sendData(jsonObject);

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

void initWifi() { // Establish a Wi-Fi connection with your router
    Serial.println("initWifi initiated");    
    Serial.println("initWifi done"); 
}

void setup()
{
    Serial.begin(9600);
    print_wakeup_reason();
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,0); //1 = High, 0 = Low

    Serial.println("Serial Up");
    SPI.begin();
    tft.init();
    Serial.println("Screen Up");
    Serial.println("Screen Up");
    tft.fillScreen(TFT_BLACK);
    Serial.println("Screen Up2");

    CreateSprites();
    ScreenStart();
    
    pinMode(buttonPin, INPUT);
    pinMode(buttonPowerPin, OUTPUT);
    digitalWrite(buttonPowerPin, HIGH);
    LastUseMillis = millis();
    LastScreenMillis = millis();
    Serial.println("Trigger Up");
    Serial.println("PlasticScanner is initialized!");

    Serial.println("6000ms delay");
    delay(6000);
    ScreenReady();
}




void loop()
{
    buttonState = digitalRead(buttonPin);
    currentMillis = millis();
    pM1 = millis();
    pM2 = millis();

    if (buttonState == LOW) {
        Serial.println("button pushed!"); 
        scan();
        LastUseMillis = currentMillis;
        LastScreenMillis = currentMillis;
        ReadyScreenMillis = currentMillis;
        AgainScreenMillis = currentMillis;

        
        Serial.println("Button done"); 
    } 

    if (currentMillis - ReadyScreenMillis >= ReadyTimer) // If no button press for some time -> sleep
    {
        ScreenReady();
        ReadyScreenMillis = currentMillis;
    }
    else if (currentMillis - AgainScreenMillis >= AgainTimer && LastResultReady) // If no button press for some time -> sleep
    {
        
        ScreenScanAgain();
        AgainScreenMillis = currentMillis;
    }

    // Place result screen here

    if (currentMillis - LastUseMillis >= SleepTimer) // If no button press for some time -> sleep
    {
        Serial.println("Going to sleep...");
        ScreenStart();
        delay(3000);
        // Place "Going to sleep screen" + delay here
        esp_deep_sleep_start();
        
    }
    
}

