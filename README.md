

# PlasticScanner-TUD
This is a branch out of the Open source [PlasticScanner project](https://github.com/Plastic-Scanner "PlasticScanner), and all work in this repo is for a project for the course Advanced embodiment design at TU Delft, Spring 2022.

## Hardware 
This firmware is ment to be used with the custom PCB for a handheld version of the PlasticScanner - the whole KiCad project is placed under "Hardware".

The firmware is coded for an ESP32-DevkitC-v4 module ([Link to Mouser](https://nl.mouser.com/ProductDetail/Espressif-Systems/ESP32-DevKitC-32D?qs=sGAEpiMZZMuqBwn8WqcFUj2aNd7i9W7u%2F%2FMnQyCeZAxGWrRMXc%2F3rA%3D%3D "ESP32-DevKitC-32D")). Other DevBoards might work aswell - but it has not been tested.
Be aware of the ESP32 being in use is the Generation 1, and not the S2 or S3 verion!

## IDE Setup 
Open the whole folder in PlatformIO, and make sure that all dependencies described in platformio.ini are pulled. 

## Pinout overview
### Power
The system runs on 5v from the ESP32 or Arduino Uno

### SPI
SPI is used for the screen and the ADC (ADS1256)
|           | ESP32   | PCB 2.1     | Description                   | 
| --------- | ------- |  ---------- | ----------------------------- |
| COPI      | 23      | MOSI        | Controller out, Pheripial in  | 
| CIPO      | 19      | MISO        | Controller in, Pheripial out  | 
| SCK       | 18      | CLK         | Clock syncronisation          |       
| CS        | 5       | CS          | Chip select                   | 
| PDWN      | 14      | PDWN        | Power down                    | 
| Reset     | 27      | Reset       | Reset                         | 
| DRDY      | 26      | DRDY        | Data Ready                    | 


OBS! there are jumper solders on the ESP to choose if the screen should use the H-SPI or V-SPI CLK and MOSI on the ESP32. 


### I2C
I2C is used for controlling the 8 specific wavelength NIR LED's
|           | ESP32   | PCB 2.1    | Description                   | 
| --------- | ------- | ---------- | ----------------------------- |
| GND       | GND     | GND        | Ground                        | 
| SDA       | 21      | SDA        | Serial Data connection        | 
| SCL       | 22      | SCL        | Serial clock                  | 
| Reset     | Nan     | Reset      | Reset                         | 


### Other
the trigger is used for - Well you guessed is - Triggering the scanner
|           | ESP32   | PCB 2.1    | Description                   | 
| --------- | ------- | ---------- | ----------------------------- |
| Trigger   | 33      | D2         | Trigger button (reset on 2.1) | 



## Known issues
- SPI communication with ADS1256 is currently not working.
- BLE notify is currently not broadcasting new values correctly 

## ToDo
- Make a simpler screen integration than TFT eSPI with user setup No. 46
- Try another ADS1256 library
- 