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
	for (int i = 0; i < size - 1; i++)
	{
		values[i + 1] = values[i];
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

	nanoseconds full_delay = 1000000ns;

	int X;	// input voltage data buffer
	int Yd; // data buffer for the filtered voltage
	static SMA<20> filter;

	float Pw[7] = {0.01, 0.25, 0.5, 1, 0.5, 0.25, 0.01};
	float Sw[7] = {0.0025, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.0025};
	float Shx[16] = {0.6011, 1.2314, 1.4398, 1.0205, -0.0293, -0.4486, 0.1817, -0.4486, -1.2874, -1.7068, -1.2874, -1.2874, -1.4984, 0.1817, 0.3927, 1.4398};
	float Shw[16] = {0.0025, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.0025};

	float Cx[16] = {0}; // the state of C(z)
	float Cw[16] = {0}; // the weight of C(z)
	float Sx[7] = {0};	// the dummy state for the secondary path
	float error;		// control error

	float controller_output;
	float Xhx[16] = {0}; // the state of the filtered x(k)
	float mu = 0.01;

	while (1)
	{
		//We need to have a steady sample rate so we can draw conclusions about the time series
		//We are going to sample at 1000Hz.
		//1/1000=0.001=1000 microseconds
		auto next = Clock::now() + full_delay;

		X = adcdac.read_adc_raw(1, 0); // Get the input voltage
		Yd = filter(X);				   // filter the voltage

		//do LMS
		shift_right(Cx, 16); // update the controller state
		Cx[0] = X;
		float Cy = dot_product(Cx, Cw, 16);
		adcdac.set_dac_raw(Cy, 1); // output anti vibration
		cout << Cy;

		shift_right(Sx, 7);
		Sx[0] = Cy;							 // propagate to secondary path
		error = Yd - dot_product(Sx, Sw, 7); //  measure the residue

		shift_right(Shx, 16); // update the state of Sh(z)
		Shx[0] = X;

		shift_right(Xhx, 16); // calculate the filtered x(k)
		Xhx[0] = dot_product(Shx, Shw, 16);

		adjust_controller_weight(Cw, Xhx, mu, error); // adjust the controller weight

		this_thread::sleep_until(next);
	}
	adcdac.close_adc();
	adcdac.close_dac();

	(void)argc;
	(void)argv;
	return 0;
}
