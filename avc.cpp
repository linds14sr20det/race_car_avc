/*
 *
 *      compile with "g++ secondary_path.cpp ABE_ADCDACPi.cpp -Wall -Wextra -Wpedantic -Woverflow -lpthread -o secondary_path"
 *      run with "./avc"
 */

#include <stdint.h>
#include <stdio.h>
#include <stdexcept>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <math.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <thread>
#include <pthread.h>
#include <random>

#include "ABE_ADCDACPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;
using Clock = std::chrono::system_clock;
using nanoseconds = std::chrono::nanoseconds;

void *ActiveVibrationControl(void *threadid) {
	ADCDACPi adcdac;

	if (adcdac.open_adc() != 1) { // open the ADC spi channel
		pthread_exit(NULL); // if the SPI bus fails to open exit the program
	}
	if (adcdac.open_dac() != 1) { // open the DAC spi channel
                pthread_exit(NULL); // if the SPI bus fails to open exit the program
        }
	
	adcdac.set_dac_gain(2);	// set the DAC gain to 2 which will give a max voltage of 3.3V
	
	float voltage;
	nanoseconds full_delay = 1000000ns;
	while (1) {
		//We need to have a steady sample rate so we can draw conclusions about the time series
		//We are going to sampe at 1000Hz.
		//1/1000=0.001=1000 microseconds 
		auto next = Clock::now() + full_delay;

		voltage = adcdac.read_adc_voltage(1, 0);

		adcdac.set_dac_voltage(voltage, 1);


		this_thread::sleep_until(next);
	}
	adcdac.close_adc();
	adcdac.close_dac();
	pthread_exit(NULL);
}

int main(int argc, char **argv) {
	//This program spawns thread to keep things fast
	pthread_t threads[1];

	pthread_create(&threads[0], NULL, ActiveVibrationControl, (void *)1);

	pthread_exit(NULL);
	(void)argc;
	(void)argv;
	return 0;
}

