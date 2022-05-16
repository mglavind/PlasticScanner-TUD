# PlasticScanner-TUD

## Pinout overview
### Power
The system runs on 5v from the ESP32 or Arduino Uno


### SPI to LED's
|           | ESP32   | PCB 2.1     | Description                   | 
| --------- | ------- |  ---------- | ----------------------------- |
| MOSI      | 23      | MOSI        | Master out, Slave in          | 
| MISO      | 19      | MISO        | Master in, Slave out          | 
| SCK       | 18      | CLK         | Clock syncronisation          |       
| CS        | Nan     | Nan         | Chip select                   | 
| PDWN      | Nan     | PDWN        | Power down                    | 
| Reset     | Nan     | Reset       | Reset                         | 
| DRDY      | Nan     | DRDY        | Data Ready                    | 


### I2C for ADC
|       | ESP32 | PCB 2.1    | Description                   | 
| ----- | ----- | ---------- | ----------------------------- |
| GND   | GND   | GND        | Ground                        | 
| SDA   | 21    | SDA        | Serial Data connection        | 
| SCL   | 22    | SCL        | Serial clock                  | 
| Reset | Nan   | Reset      | Reset                         | 


### SPI for Display
|           | ESP32   | Display     | Description                   | 
| --------- | ------- |  ---------- | ----------------------------- |
| MOSI      | 23      | MOSI        | Master out, Slave in          | 
| MISO      | Not used | Not used   | Master in, Slave out          | 
| SCK       | 18      | CLK         | Clock syncronisation          |       
| CS        | 5       | CS          | Chip select                   | 
| DC        | 27      | DC          | No idea                       | 
| Reset     | 33      | RST         | Reset                         | 
| BL        | 22      | BL          | Backlight control             | 

Note: MISO is not used, since the screen has no output.

WE NEED TO FIND OUT THE CS OF THE LED'S!!!!  


