#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "ABE_ADCDACPi.h"

int main(int argc, char **argv){
	setvbuf (stdout, NULL, _IONBF, 0); // needed to print to the command line

	if (open_adc() != 1){ // open the ADC spi channel
		printf("Failed to open the ADC.\n");	
		exit(1); // if the SPI bus fails to open exit the program
	}

	if (open_dac() != 1){ // open the DAC spi channel
		printf("Failed to open the DAC.\n");
		exit(1); // if the SPI bus fails to open exit the program
	}

	set_dac_gain(2);

	while (1){
		// Set the output voltage of channel 2 to the input voltage of channel 1
		set_dac_voltage(read_adc_voltage(1, 0), 2);
	}

	(void)argc;
	(void)argv;
	return (0);
}
