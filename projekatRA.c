#include <stdio.h>
#include <bcm2835.h>
#include "lcd1602.h"

#define PIN 21
#define WET 292
#define DRY 730
#define RANGE (DRY-WET)

uint8_t start = 0x01; //start byte
uint8_t end = 0x00;
uint8_t chan = 0x00; //channel in this case we select CH0
int adc = 0;
char str[3];
int percent;

int readADC(uint8_t chan){ //reads analog value from adc using spi
	char buf[] = {start, (0x08|chan)<<4, end}; //3 bytes 
	char readBuf[3];
	bcm2835_spi_transfernb(buf, readBuf, 3);
	return ((int)readBuf[1] & 0x03) << 8 | (int)readBuf[2];//return 2 last bits from readBuf[1] and 8 bits from readBuf[2]
}	

int soilMoisture(int ADC){ //returns percentage of soil moisture
	int moisture = (int)((DRY-ADC)/(RANGE/100));
	if(moisture>100)
		return 100;
	if(moisture<0)
		return 0;
	return moisture;
}

int main (int argc, char const *argv[]){
	if(!bcm2835_init()){
		printf("bcm2835_init failed\n");
		return 1;
	}

	if(!bcm2835_spi_begin()){
		printf("bcm2835_spi_begin failed\n");
	}

	int rc;
	rc = lcd1602Init(1,0x27);
	if(rc){
		printf("lcd1602Init failed\n");
		return 1;
	}
	
	//SPI configuration
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
	
	//GPIO configuration 
	bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_OUTP);
	
	while(1){
		bcm2835_gpio_write(PIN, LOW);
		bcm2835_delay(5000);

		adc = readADC(chan); //read value from sensor
		percent = soilMoisture(adc); //converts to percentage from 0 to 100

		sprintf(str, "%d", percent);
		lcd1602Clear();
		lcd1602SetCursor(0,0);
		lcd1602WriteString("Vlaznost zemlje:");
		lcd1602SetCursor(0,1);
		lcd1602WriteString(str);
		lcd1602WriteString("%");
		printf("Vlaznost zemljista je:  %d% \n", percent);

		if(percent<20){
			bcm2835_gpio_write(PIN, HIGH);
			bcm2835_delay(2000);
		}
//		else{
//			bcm2835_gpio_write(PIN, LOW);
//			bcm2835_delay(5000);
//					}
	}
}
