/*
 *
 *      compile with "g++ avc.cpp ../ABE_ADCDACPi.cpp -Wall -Wextra -Wpedantic -Woverflow -o avc"
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

#include "DspFilters/Dsp.h"
#include "ABE_ADCDACPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;

void clearscreen()
{
	printf("\033[2J\033[1;1H");
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0); // needed to print to the command line

	ADCDACPi adcdac;

	if (adcdac.open_adc() != 1)
	{				// open the ADC spi channel
		return (1); // if the SPI bus fails to open exit the program
	}

	int numSamples = 2000;
	float* input[1];
	input[0] = new float[numSamples];

	Dsp::Filter* f = new Dsp::SmoothedFilterDesign<Dsp::Butterworth::Design::BandPass <4>, 1, Dsp::DirectFormII> (1024);
    	Dsp::Params params;
    	params[0] = 44100; // sample rate
    	params[1] = 4; // order
    	params[2] = 4000; // center frequency
    	params[3] = 880; // band width
    	f->setParams (params);
    	f->process (numSamples, input);

	ofstream myfile;
	myfile.open("log.txt");
	float voltage;

	while (1)
	{
		auto begin = std::chrono::high_resolution_clock::now();

		voltage = adcdac.read_adc_voltage(1, 0);
		myfile << voltage << endl;

		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		auto elapsed_micro = elapsed.count() * 1e-3;
		usleep(1000 - elapsed_micro); // sleep 1000 microseconds
	}

	myfile.close();
	(void)argc;
	(void)argv;
	return (0);
}

