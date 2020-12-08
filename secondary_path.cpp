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

#include "ABE_ADCDACPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;
using Clock = std::chrono::system_clock;
using nanoseconds = std::chrono::nanoseconds;

void *GenerateSineWave(void *threadid) {
	ADCDACPi adcdac;

	if (adcdac.open_dac() != 1) { // open the DAC spi channel
		pthread_exit(NULL); // if the SPI bus fails to open exit the program
	}

	adcdac.set_dac_gain(2); // set the dac gain to 1 which will give a voltage range of 0 to 2.048V

	uint16_t DACLookup[16] = {2048,2831,3495,3939,4095,3939,3495,2831,2048,1264,600,156,0,156,600,1264};
	uint16_t i;
	nanoseconds full_delay = 462962ns;

	while (1) {
		//We're going to produce a 135Hz wave.
		//We have 16 points per cycle. Each cycle needs to complete in 7407407 nanoseconds (1e+9/135=7407407)
		//This means each point needs to run in 462962 nanoseconds.
		for (i=0; i < 16; i = i + 1) {
			auto next = Clock::now() + full_delay;
			adcdac.set_dac_raw(DACLookup[i], 1);
			this_thread::sleep_until(next);
		}
		i = 0;
	}

	adcdac.close_dac();
	pthread_exit(NULL);
}

void *GenerateNoise(void *threadid) {
	ADCDACPi adcdac;

	if (adcdac.open_dac() != 1) { // open the DAC spi channel
		pthread_exit(NULL); // if the SPI bus fails to open exit the program
	}

	adcdac.set_dac_gain(2); // set the dac gain to 1 which will give a voltage range of 0 to 2.048V

	nanoseconds full_delay = 200000ns;
	std::default_random_engine generator;
  	std::normal_distribution<double> distribution(2047.0, 680.0);

	while (1) {
		auto next = Clock::now() + full_delay;
		//We're going to produce totally random noise.
		adcdac.set_dac_raw(int(distribution(generator)), 1);
		this_thread::sleep_until(next);
	}

	adcdac.close_dac();
	pthread_exit(NULL);
}

void *RecordNoise(void *threadid) {
	ADCDACPi adcdac;

	if (adcdac.open_adc() != 1) { // open the ADC spi channel
		pthread_exit(NULL); // if the SPI bus fails to open exit the program
	}
	
	uint i = 0;
	ofstream tmpfile;
	tmpfile.open("log.txt");
	float voltage;
	nanoseconds full_delay = 1000000ns;
	while (1) {
		//We need to have a steady sample rate so we can draw conclusions about the time series
		//We are going to sampe at 1000Hz.
		//1/1000=0.001=1000 microseconds 
		auto next = Clock::now() + full_delay;

		voltage = adcdac.read_adc_voltage(1, 0);
		tmpfile << voltage << endl;

		this_thread::sleep_until(next);
		i++;
		//This thing is sampling really fast and writing to a file. This could corrupt system memory if it gets too big
		if(i>100000) {
			break;
		}
	}
	adcdac.close_adc();
	tmpfile.close();
	pthread_exit(NULL);
}

int main(int argc, char **argv) {
	//This program spawns thread to keep things fast
	pthread_t threads[2];

	//pthread_create(&threads[0], NULL, GenerateSineWave, (void *)0);
	pthread_create(&threads[0], NULL, GenerateNoise, (void *)0);
	pthread_create(&threads[1], NULL, RecordNoise, (void *)1);

	pthread_exit(NULL);
	(void)argc;
	(void)argv;
	return 0;
}

