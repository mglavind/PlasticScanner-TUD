# PlasticScanner-TUD

## Pinout overview
### Power
The system runs on 5v from the ESP32 or Arduino Uno

Hello from Jeppe!

### SPI
|           | ESP32   | PCB 2.1     | Description                   | 
| --------- | ------- |  ---------- | ----------------------------- |
| MOSI      | 23      | MOSI        | Master out, Slave in          | 
| MISO      | 19      | MISO        | Master in, Slave out          | 
| SCK       | 18      | CLK         | Clock syncronisation          |       
| CS        | 10      | CS          | Chip select                   | 
| PDWN      | Nan     | PDWN        | Power down                    | 
| Reset     | 8       | Reset       | Reset                         | 
| DRDY      | 9       | DRDY        | Data Ready                    | 


### I2C
|       | ESP32 | PCB 2.1    | Description                   | 
| ----- | ----- | ---------- | ----------------------------- |
| GND   | GND   | GND        | Ground                        | 
| SDA   | 21    | SDA        | Serial Data connection        | 
| SCL   | 22    | SCL        | Serial clock                  | 
| Reset | Nan   | Reset      | Reset                         | 




### old SPI
|           | ESP32   | PCB 2.1     | Description                   | 
| --------- | ------- |  ---------- | ----------------------------- |
| MOSI      | 23      | MOSI        | Master out, Slave in          | 
| MISO      | 19      | MISO        | Master in, Slave out          | 
| SCK       | 18      | CLK         | Clock syncronisation          |       
| CS        | 10      | CS          | Chip select                   | 
| PDWN      | Nan     | PDWN        | Power down                    | 
| Reset     | 8       | Reset       | Reset                         | 
| DRDY      | 9       | DRDY        | Data Ready                    | 

