#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "ABE_ADCDACPi.h"



void clearscreen ()
{
    printf("\033[2J\033[1;1H");
}

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

	float voltage = 0;
	FILE *fptr = fopen("log.txt", "w");
	
	clock_t t;
	double cpu_time_used;	
	
	while (1) {
		t = clock();
		voltage = read_adc_voltage(1, 0);
		fprintf(fptr, "%f\n", voltage);
		t = clock() - t;
		cpu_time_used = ((double)t)/(CLOCKS_PER_SEC/1000);
		usleep(50 - cpu_time_used);	
		//set_dac_voltage(voltage, 1);
		//if (voltage > max) {
		//	max = voltage;
		//}
		//if (voltage < min) {
		//	min = voltage;
		//}
		//clearscreen();		
		//printf("Max voltage: %G \n", max);
		//printf("Min voltage: %G \n", min);
		//printf("Voltage: %G \n", voltage);
	}
	fclose(fptr);
	(void)argc;
	(void)argv;
	return (0);
}
