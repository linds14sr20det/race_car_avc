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

#include "ABE_ADCDACPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;

void clearscreen()
{
	printf("\033[2J\033[1;1H");
}

double butterworth(double x, int n, double s, double f)
{
	int i = n;
	n = n / 2;
	double a = tan(M_PI * f / s);
	double a2 = a * a;
	double r;
	double *A = (double *)malloc(n * sizeof(double));
	double *d1 = (double *)malloc(n * sizeof(double));
	double *d2 = (double *)malloc(n * sizeof(double));
	double *w0 = (double *)calloc(n, sizeof(double));
	double *w1 = (double *)calloc(n, sizeof(double));
	double *w2 = (double *)calloc(n, sizeof(double));

	for (i = 0; i < n; ++i)
	{
		r = sin(M_PI * (2.0 * i + 1.0) / (4.0 * n));
		s = a2 + 2.0 * a * r + 1.0;
		A[i] = a2 / s;
		d1[i] = 2.0 * (1 - a2) / s;
		d2[i] = -(a2 - 2.0 * a * r + 1.0) / s;
	}

	for (i = 0; i < n; ++i)
	{
		w0[i] = d1[i] * w1[i] + d2[i] * w2[i] + x;
		x = A[i] * (w0[i] + 2.0 * w1[i] + w2[i]);
		w2[i] = w1[i];
		w1[i] = w0[i];
	}
	return x;
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0); // needed to print to the command line

	ADCDACPi adcdac;

	if (adcdac.open_adc() != 1)
	{				// open the ADC spi channel
		return (1); // if the SPI bus fails to open exit the program
	}

	ofstream myfile;
	myfile.open("log.txt");
	float voltage;

	while (1)
	{
		auto begin = std::chrono::high_resolution_clock::now();

		voltage = adcdac.read_adc_voltage(1, 0);
		double butterworthed = butterworth(voltage, 6, 20000, 10000);
		myfile << butterworthed << endl;

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

