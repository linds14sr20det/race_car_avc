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

std::atomic<bool> read_input(true);
std::atomic<bool> log_output(false);

void ReadUserInput()
{
	char user_input;
	while (read_input.load())
	{
		cout << "Waiting for user input (l for turning on logging, e to exit):\n";
		std::cin >> user_input;
		switch (user_input)
		{
		case 'e':
			read_input.store(false);
			break;
		case 'l':
			log_output.store(!log_output.load());
			break;
		}
	}
}

float convert_raw_to_voltage(int raw)
{
	return float(raw) * 3.3 / 4095;
}

void ActiveVibrationControl()
{
	//Define constants
	int N = 25;	   //Filter length
	float mu = 25; //Define LMS step-size

	//Define 'for' loop counters
	int k = 0; //Stored reference sample counter
	int i = 0; //Convolution counter

	//Vectors, to implement matrix multiplication
	float w[25];		//Adaptive filter coefficients
	float x_window[25]; //Define stored x values used in convolution

	float x_biased;
	float e_biased;
	float x;
	float e;
	float y;
	int log_count = 0;

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

	ofstream tmpfile;
	tmpfile.open("log.txt");
	tmpfile << "Timestamp, Engine Vibration, Chassis Vibration (error), Output" << endl;

	nanoseconds full_delay = 1000000ns;
	auto start = Clock::now();

	while (read_input.load())
	{
		log_count++;
		//We need to have a steady sample rate so we can draw conclusions about the time series
		//We are going to sample at 1000Hz.
		//1/1000=0.001=1000 microseconds
		auto next = Clock::now() + full_delay;

		//Preliminary signals
		x_biased = adcdac.read_adc_voltage(1, 0); //Get biased input engine vibration
		e_biased = adcdac.read_adc_voltage(2, 0); //Get biased input chassis vibration (error)
		x = (x_biased - 1.65);					  //Unbias reference signal to obtain original recorded x
		e = (e_biased - 1.65);					  //Unbias error signal to obtain original recorded e

		//Populate stored reference value matrix
		for (k = N - 1; k > -1; k--)
		{ //Shift values right, such that most recent sample is x_window[0]
			if (k == 0)
			{
				x_window[k] = x; //Most recently sampled value assigned to first entry of stored reference array
			}
			else
			{
				x_window[k] = x_window[k - 1]; //Shift right
			}
		}

		//Perform LMS
		y = 0; //Prepare y value for convolution
		for (i = 0; i < N; i++)
		{											  //Loop for every value in the 'matrices'
			y = y + (w[i]) * (x_window[i]);			  //Convolution implementation
			w[i] = w[i] + (2 * mu * e * x_window[i]); //Update filter coefficients
		}

		//Output after biasing for DAC
		adcdac.set_dac_voltage(y + 1.65, 1); // output anti vibration
		adcdac.set_dac_voltage(y + 1.65, 2); // output anti vibration

		if (log_output.load() && log_count < 100000)
		{
			tmpfile << std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count() << "," << x_biased << "," << e_biased << "," << y << endl;
		}

		this_thread::sleep_until(next);
	}
	tmpfile.close();
	adcdac.close_adc();
	adcdac.close_dac();
}

int main(int argc, char **argv)
{
	std::thread i_o(ReadUserInput);
	std::thread avc(ActiveVibrationControl);

	i_o.join();
	avc.join();

	(void)argc;
	(void)argv;
	return 0;
}
