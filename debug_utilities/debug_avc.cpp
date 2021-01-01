/*
 *
 *      compile with "g++ debug_avc.cpp ABE_ADCDACPi.cpp -Wall -Wextra -Wpedantic -Woverflow -o debug_avc"
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
#include <mutex>
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
		cout << "Waiting for user input:\n";
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
	pthread_exit(NULL);
}

void ReadAdc()
{
	ADCDACPi adcdac;

	if (adcdac.open_adc() != 1)
	{						// open the ADC spi channel
		pthread_exit(NULL); // if the SPI bus fails to open exit the program
	}
	if (adcdac.open_dac() != 1)
	{						// open the DAC spi channel
		pthread_exit(NULL); // if the SPI bus fails to open exit the program
	}

	adcdac.set_dac_gain(2); // set the DAC gain to 2 which will give a max voltage of 3.3V

	ofstream tmpfile;
	tmpfile.open("debug_log.txt");
	tmpfile << "Timestamp, Engine Raw, Chassis Raw, Output" << endl;

	nanoseconds full_delay = 1000000ns;

	float engine_vibration;	 // input voltage data buffer
	float chassis_vibration; // input voltage data buffer
	float Y;
	float amplitude_weight = 1.5;

	auto start = Clock::now();
	while (read_input.load())
	{
		//We need to have a steady sample rate so we can draw conclusions about the time series
		//We are going to sample at 1000Hz.
		//1/1000=0.001=1000 microseconds
		auto next = Clock::now() + full_delay;

		engine_vibration = adcdac.read_adc_voltage(1, 0);  // Get the input voltage
		chassis_vibration = adcdac.read_adc_voltage(2, 0); // Get the "error" input voltage
		Y = ((engine_vibration - 1.69) * -1 * amplitude_weight) + 1.69;

		if (output_dac.load())
		{
			adcdac.set_dac_voltage(Y, 1); // output anti vibration
		}
		tmpfile << std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count() << "," << engine_vibration << "," << chassis_vibration << "," << Y << endl;

		this_thread::sleep_until(next);
	}

	tmpfile.close();
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
