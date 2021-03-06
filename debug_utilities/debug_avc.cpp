/*
 *      compile with "g++ debug_avc.cpp ABE_ADCDACPi.cpp -Wall -Wextra -Wpedantic -Woverflow -o debug_avc"
 *      run with "./debug_avc"
 * 
 * 		This program runs a very simple AVC algorithm. It effectively measures the vibration of the engine via 
 * 		a sensor mounted to the gearbox, while the error is measured via a sensor mounted to the transmission
 * 		mount. This is then logged. The output algorithm effectively inverses the engine vibration and sends it
 * 		to the actuator. This is a very naive approach only used for debugging. The end user can toggle the 
 * 		vibration control on and off via the console, which is useful to measure the output effect on the 
 * 		system.
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
#include <string>
#include <atomic>

#include "ABE_ADCDACPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;
using Clock = std::chrono::system_clock;
using nanoseconds = std::chrono::nanoseconds;

std::atomic<bool> read_input(true);
std::atomic<bool> output_dac(false);

void ReadUserInput()
{
	char user_input;
	while (read_input.load())
	{
		cout << "Waiting for user input (o for turn on output, e to exit):\n";
		std::cin >> user_input;
		switch (user_input)
		{
		case 'e':
			read_input.store(false);
			break;
		case 'o':
			output_dac.store(!output_dac.load());
			break;
		}
	}
}

void ReadAdc()
{
	ADCDACPi adcdac;

	if (adcdac.open_adc() != 1) // open the ADC spi channel
	{
		return; // if the SPI bus fails to open exit the program
	}
	if (adcdac.open_dac() != 1) // open the DAC spi channel
	{
		return; // if the SPI bus fails to open exit the program
	}

	adcdac.set_dac_gain(2); // set the DAC gain to 2 which will give a max voltage of 3.3V

	nanoseconds full_delay = 2000000ns;

	float engine_vibration;	 // input voltage data buffer
	float Y;

	while (read_input.load())
	{
		//We need to have a steady sample rate so we can draw conclusions about the time series
		//We are going to sample at 1000Hz.
		//1/1000=0.001=1000 microseconds
		auto next = Clock::now() + full_delay;

		engine_vibration = adcdac.read_adc_voltage(2, 0);  // Get the input voltage
		Y = 1.68;
		if (output_dac.load())
		{
			Y = ((engine_vibration - 1.69) * -1 * 2.7) + 1.69;
			adcdac.set_dac_voltage(Y, 1); // output anti vibration
			adcdac.set_dac_voltage(Y, 2); // output anti vibration
		}

		this_thread::sleep_until(next);
	}

	adcdac.close_adc();
	adcdac.close_dac();
}

int main()
{
	std::thread i_o(ReadUserInput);
	std::thread adcdac(ReadAdc);

	i_o.join();
	adcdac.join();
	return 0;
}
