/*
 *
 *      compile with "g++ avc.cpp ABE_ADCDACPi.cpp MiniPid.cpp -Wall -Wextra -Wpedantic -Woverflow -lpthread -o avc"
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
#include <atomic>

#include "ABE_ADCDACPi.h"
#include "MiniPID.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;
using Clock = std::chrono::system_clock;
using nanoseconds = std::chrono::nanoseconds;

std::atomic<bool> read_input(true);
std::atomic<bool> log_output(false);
std::atomic<bool> controller_output(false);
std::atomic<float> gain(2.0);

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
		case 'o':
			controller_output.store(!controller_output.load());
			break;
		case '+':
			gain.store(gain.load() + 0.5);
			cout << gain << "\n";
			break;
		case '-':
			gain.store(gain.load() - 0.5);
			cout << gain << "\n";
			break;
		}
	}
}

void ActiveVibrationControl()
{
	float x_biased;
	float e_biased;
	float x;
	float e = 0;
	float y;
	float y_adjusted = 0;
	int log_count = 0;

	ADCDACPi adcdac;
	MiniPID pid = MiniPID(1, 0, 0);
	pid.setOutputLimits(-1.64, 1.64);
	pid.setOutputRampRate(10);

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
	tmpfile << "Timestamp, Engine Vibration, Anti Vibration Signal, Chassis Vibration (error)" << endl;

	nanoseconds full_delay = 1000000ns;
	auto start = Clock::now();

	while (read_input.load())
	{
		//We need to have a steady sample rate so we can draw conclusions about the time series
		//We are going to sample at 1000Hz.
		//1/1000=0.001=1000 microseconds
		auto next = Clock::now() + full_delay;

		//Preliminary signals
		x_biased = adcdac.read_adc_voltage(2, 0); //Get biased input engine vibration

		x = (x_biased - 1.704); //Unbias reference signal to obtain original recorded x

		//y_adjusted = pid.getOutput(e, y);

		y = 0;
		if (controller_output.load())
		{
			y = gain.load() * x;
			adcdac.set_dac_voltage(y + 1.645, 1); // output anti vibration
			adcdac.set_dac_voltage(y + 1.645, 2); // output anti vibration
		}

		e_biased = adcdac.read_adc_voltage(1, 0); //Get biased input chassis vibration (error)
		e = (e_biased - 1.692);					  //Unbias error signal to obtain original recorded e

		if (log_output.load() && log_count < 100000)
		{
			log_count++;
			tmpfile << std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - start).count() << "," << x << "," << y << "," << e << endl;
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
