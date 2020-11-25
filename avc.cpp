/*
 * demo-adcread.cpp
 *
 *  Version 1.1 Updated 21/04/2020
 *
 *      compile with "g++ demo-adcread.cpp ../ABE_ADCDACPi.cpp -Wall -Wextra -Wpedantic -Woverflow -o demo-adcread"
 *      run with "./demo-adcread"
 */

#include <stdint.h>
#include <stdio.h>
#include <stdexcept>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <fstream>

#include "ABE_ADCDACPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;

void clearscreen ()
{
    printf("\033[2J\033[1;1H");
}

int main(int argc, char **argv){
	setvbuf (stdout, NULL, _IONBF, 0); // needed to print to the command line

	ADCDACPi adcdac;

	if (adcdac.open_adc() != 1){ // open the ADC spi channel
		return (1); // if the SPI bus fails to open exit the program
	}

	ofstream myfile;
  	myfile.open ("log.txt");
	float voltage;

	while (1){
		auto begin = std::chrono::high_resolution_clock::now();

		voltage = adcdac.read_adc_voltage(1, 0);
  		myfile << voltage << endl;

		auto end = std::chrono::high_resolution_clock::now();
    		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		auto elapsed_micro = elapsed.count() * 1e-3;
		usleep(1000-elapsed_micro); // sleep 1000 microseconds
	}

  	myfile.close();
	(void)argc;
	(void)argv;
	return (0);
}

