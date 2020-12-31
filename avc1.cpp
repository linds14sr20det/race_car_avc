/*
 *
 *      compile with "g++ avc.cpp ABE_ADCDACPi.cpp -Wall -Wextra -Wpedantic -Woverflow -o avc"
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
#include <random>

#include "ABE_ADCDACPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;
using Clock = std::chrono::system_clock;
using nanoseconds = std::chrono::nanoseconds;

void adjust_controller_weight(float Cw[], float Xhx[], float mu, float error)
{
	for (int i = 0; i < 16; i++)
	{
		Cw[i] = Cw[i] + Xhx[i] * mu * error;
	}
}

void shift_right(float values[], int size)
{
	for (int i = size - 1; i > -1; i--)
	{
		values[i] = values[i-1];
	}
}

float dot_product(float vector_a[], float vector_b[], int size)
{
	float product = 0;
	for (int i = 0; i < size; i++)
		product = product + vector_a[i] * vector_b[i];
	return product;
}

template <uint8_t N, class input_t = uint16_t, class sum_t = uint32_t>
class SMA
{
public:
	input_t operator()(input_t input)
	{
		sum -= previousInputs[index];
		sum += input;
		previousInputs[index] = input;
		if (++index == N)
			index = 0;
		return (sum + (N / 2)) / N;
	}

	static_assert(
		sum_t(0) < sum_t(-1), // Check that `sum_t` is an unsigned type
		"Error: sum data type should be an unsigned integer, otherwise, "
		"the rounding operation in the return statement is invalid.");

private:
	uint8_t index = 0;
	input_t previousInputs[N] = {};
	sum_t sum = 0;
};

float convert_raw_to_voltage(int raw) 
{
	return float(raw) * 3.3 / 4095;
}

int main(int argc, char **argv)
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
	tmpfile.open("log.txt");
	tmpfile << "Timestamp, Engine Raw, Chassis Raw, Output" << endl;
	
	nanoseconds full_delay = 1000000ns;

	float engine_vibration;	// input voltage data buffer
	float chassis_vibration;	// input voltage data buffer
	float Y;
	float amplitude_weight = 3.5;
	float mu = 0.3;
	
	auto start = Clock::now();
	while (1)
	{
		//We need to have a steady sample rate so we can draw conclusions about the time series
		//We are going to sample at 1000Hz.
		//1/1000=0.001=1000 microseconds
		auto next = Clock::now() + full_delay;

		engine_vibration = adcdac.read_adc_voltage(1, 0); // Get the input voltage
		chassis_vibration = adcdac.read_adc_voltage(2, 0); // Get the "error" input voltage
		Y = ((engine_vibration-1.69) * -1 * amplitude_weight) + 1.69;

		adcdac.set_dac_voltage(Y, 1); // output anti vibration

		tmpfile << std::chrono::duration_cast<std::chrono::nanoseconds> (Clock::now() - start).count() << "," << engine_vibration << "," << chassis_vibration << "," << Y  << endl;
		


		this_thread::sleep_until(next);
	}
	tmpfile.close();
	adcdac.close_adc();
	adcdac.close_dac();

	(void)argc;
	(void)argv;
	return 0;
}
