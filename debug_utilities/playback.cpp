/*
 *
 *      compile with "g++ playback.cpp ABE_ADCDACPi.cpp -Wall -Wextra -Wpedantic -Woverflow -o playback"
 *      run with "./playback"
 * 
 * 		This program is useful to debug that the physical system is actually working. This effectively
 * 		"plays" a vibration recording back via the DAC output. This playback consists of an engine idling,
 * 		then revved to 4500 rpm (resonance in the chassis at this rpm point), then back to idle, then
 * 		back to 4500 rpm. There is no vibration control implemented here.
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

#include "ABE_ADCDACPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;
using Clock = std::chrono::system_clock;
using nanoseconds = std::chrono::nanoseconds;

int main(int argc, char **argv)
{
	ADCDACPi adcdac;

	if (adcdac.open_dac() != 1)
	{						// open the DAC spi channel
		pthread_exit(NULL); // if the SPI bus fails to open exit the program
	}

	adcdac.set_dac_gain(2); // set the DAC gain to 2 which will give a max voltage of 3.3V

	/* Creating input filestream */
	ifstream output_file("output_for_playback.txt");
	string line;

	nanoseconds full_delay = 1000000ns;

	float voltage;
	while (1)
	{
		//We need to have a steady sample rate so we can draw conclusions about the time series
		//We are going to sample at 1000Hz.
		//1/1000=0.001=1000 microseconds
		auto next = Clock::now() + full_delay;
		getline(output_file, line);
		voltage = stof(line);
		adcdac.set_dac_voltage(voltage, 1); // output anti vibration
		adcdac.set_dac_voltage(voltage, 2); // output anti vibration

		this_thread::sleep_until(next);
	}
	adcdac.close_dac();

	(void)argc;
	(void)argv;
	return 0;
}
