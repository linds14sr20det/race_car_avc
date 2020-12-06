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
#include <pthread.h>

#include "ABE_ADCDACPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;

void *GenerateNoise(void *threadid) {
	ADCDACPi adcdac;

	if (adcdac.open_dac() != 1) { // open the DAC spi channel
		pthread_exit(NULL); // if the SPI bus fails to open exit the program
	}

	adcdac.set_dac_gain(1); // set the dac gain to 1 which will give a voltage range of 0 to 2.048V

	uint16_t DACLookup[256] = {2048,2098,2148,2198,2248,2298,2348,2398,2447,2496,2545,2594,2642,2690,2737,2784,2831,2877,2923,2968,3013,3057,3100,3143,3185,3226,3267,3307,3346,3385,3423,3459,3495,3530,3565,3598,3630,3662,3692,3722,3750,3777,3804,3829,3853,3876,3898,3919,3939,3958,3975,3992,4007,4021,4034,4045,4056,4065,4073,4080,4085,4089,4093,4094,4095,4094,4093,4089,4085,4080,4073,4065,4056,4045,4034,4021,4007,3992,3975,3958,3939,3919,3898,3876,3853,3829,3804,3777,3750,3722,3692,3662,3630,3598,3565,3530,3495,3459,3423,3385,3346,3307,3267,3226,3185,3143,3100,3057,3013,2968,2923,2877,2831,2784,2737,2690,2642,2594,2545,2496,2447,2398,2348,2298,2248,2198,2148,2098,2048,1997,1947,1897,1847,1797,1747,1697,1648,1599,1550,1501,1453,1405,1358,1311,1264,1218,1172,1127,1082,1038,995,952,910,869,828,788,749,710,672,636,600,565,530,497,465,433,403,373,345,318,291,266,242,219,197,176,156,137,120,103,88,74,61,50,39,30,22,15,10,6,2,1,0,1,2,6,10,15,22,30,39,50,61,74,88,103,120,137,156,176,197,219,242,266,291,318,345,373,403,433,465,497,530,565,600,636,672,710,749,788,828,869,910,952,995,1038,1082,1127,1172,1218,1264,1311,1358,1405,1453,1501,1550,1599,1648,1697,1747,1797,1847,1897,1947,1997};

	uint16_t i;

	while (1) {
		//We're going to produce a 135Hz wave.
		//We have 256 points per cycle. Each cycle needs to complete in 7407 microseconds (1000000/135=7407.4)
		//This means each point needs to run in 28.9 microseconds.
		for (i=0; i < 256; i = i + 1) {
			auto begin = std::chrono::high_resolution_clock::now();
			adcdac.set_dac_raw(DACLookup[i],1);
			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
			auto elapsed_micro = elapsed.count() * 1e-3;
			usleep(29 - elapsed_micro);
		}
		i = 0;
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
	while (1) {
		//We need to have a steady sample rate so we can draw conclusions about the time series
		//We are going to sampe at 1000Hz.
		//1/1000=0.001=1000 microseconds 
		auto begin = std::chrono::high_resolution_clock::now();

		voltage = adcdac.read_adc_voltage(1, 0);
		tmpfile << voltage << endl;

		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		auto elapsed_micro = elapsed.count() * 1e-3;
		usleep(1000 - elapsed_micro);
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

	pthread_create(&threads[0], NULL, GenerateNoise, (void *)0);
	pthread_exit(NULL);
	(void)argc;
	(void)argv;
	return 0;
}

