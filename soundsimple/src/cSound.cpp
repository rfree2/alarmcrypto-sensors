/*
 * cSound.cpp
 *
 *  Created on: 17 lis 2014
 *      Author: abby
 */

#include "cSound.h"
#include "gnuplot_i.hpp"

using namespace std;

cSound::cSound(bool s) : simulation_(s)
{
	_info("constructor");
}

cSound::~cSound() {
	_dbg3("destructor");
}

void cSound::ProccessRecording(const sf::Int16* Samples, std::size_t SamplesCount, unsigned int SampleRate) {
	//cout << "Processing chunk of size " << SamplesCount << endl;
	_fact("Proccessing recording");
	_info(*Samples << " , count " << SamplesCount);
	//unsigned int SampleRate = GetSampleRate();

	// Convert input array to double
	double *in = new double[SamplesCount];
	for (size_t i = 0; i < SamplesCount; i++) {
		in[i] = static_cast<double>(Samples[i]);
	}

	size_t N; // FFT size
	N = SamplesCount;

	fftw_complex *out;
	fftw_plan p;
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
	p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
	fftw_execute(p);

	// Calculate array of frequencies
	std::vector<double> freq;
	double f = 0;
	for (size_t i = 0; i < N; i++) {
		f += (1.0 / N) * SampleRate;
		freq.push_back(f);
	}

	//Calculate magnitude
	double r, i, magOne;
	std::vector<double> mag;
	double maxMag = 0;
	for (size_t ind = 0; ind < N; ind++) {
		i = out[ind][0];
		r = out[ind][1];
		magOne = sqrt((r * r) + (i * i));
		mag.push_back(magOne); // simple filter
		if (mag.at(ind) > maxMag) {
			maxMag = mag.at(ind);
		}
	}
	double threshold = 0.5; // magnitude threshold

	//Normalize
	for (size_t i = 0; i < N; i++) {
		mag.at(i) /= maxMag;

		if(mag.at(i) >= threshold) _dbg1(mag.at(i) << "\t" << freq.at(i));
	}

	//Detect Alarm


	int detectLow = 4500;
	int detectHigh = 5500;

	int LowInd = detectLow / ((1.0 / N) * SampleRate); _dbg3("Low: " << LowInd);
	int HighInd = detectHigh / ((1.0 / N) * SampleRate); _dbg3("High: " << HighInd);

	vector<double> magRange;
	vector<double> magX;

	int confirmations = 0;

	// ==
	vector<double> freqs;

	// ==
	double energy = 0.;

	for (int i = LowInd; i < HighInd; i++) {
		energy += mag.at(i);
		if (mag.at(i) > threshold) {
			confirmations++;
			freqs.push_back(i);
		}
		magRange.push_back(mag.at(i));
		magX.push_back(freq.at(i));
	}
	_dbg2("energy (sum of mags): " << energy);


	if (confirmations ) {
		_mark("alarm: " << confirmations);

		std::ofstream log;
		log.open("log.txt", std::ios::app);
		cout << currentDateTime() << "  ALARM DETECTED" << endl;
		log << currentDateTime() << "  ALARM DETECTED" << endl;
		log.close();
	}


	// Plot results
	bool plot;
	plot = 1;

	if (plot) {
		try {
			Gnuplot g1("spectrum");

			cout << endl << endl << "spectrum plot" << endl;
			g1.set_grid();
			g1.set_style("points").plot_xy(freq, mag, " ");

			//g1.set_style("points").plot_xy(magX,magRange,"magnitude");

			if(simulation_) wait_for_key();
			else sleep(2);
		}
		catch (GnuplotException & ge) {
			cout << ge.what() << endl;
		}
	}

	fftw_destroy_plan(p);
	fftw_free(in);
	fftw_free(out);

	cout << "\n\n\n" << endl;

}

void cSound::ProccessRecordingTest(const sf::SoundBuffer Buffer) {

}

const std::string cSound::currentDateTime() {
	time_t now = time(0);
	struct tm tstruct;
	char buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	return buf;
}

void cSound::wait_for_key() {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)  // every keypress registered, also arrow keys
	cout << endl << "Press any key to continue..." << endl;

	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	_getch();
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
	cout << endl << "Press ENTER to continue..." << endl;

	std::cin.clear();
	std::cin.ignore(std::cin.rdbuf()->in_avail());
	std::cin.get();
#endif
	return;
}