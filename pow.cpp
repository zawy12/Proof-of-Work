#include <iostream>
#include <vector>
#include <fstream> 
#include <string> 
#include <math.h>
#include <chrono>
#include <stdint.h>
using namespace std; 

// Compiled with  g++ -std=c++11

// Copyright 2018 Zawy, MIT License
// This is a POW. 
// The goal is to have a simple, random, non-optimizable algorithm that 
// requires heavy processor use. The algorithm changes every nonce. 
// RAM or large cache should not make it faster, or my goal has 
// not been acheived. My belief is that ASICS, FPGAs, GPUs, CPUs, and 
// cell phones are on a more equal footing with this in terms of 
// hashes per Watt because the simpe math functions should already be 
// optimized, and this random combination of them should not be 
// mathematically optimizable.
// 
// What it does:  
// k = sha265(nonce + previous hash)  // modeled but not included here.
// Use top 12 decimal digits of k to design algorithm and seed.
// Algorithm is a sequence of 11 possible math functions used exactly twice.
// Number of possible algorithms is a little over (11!)^2 = 1E15.
// Algorithm iterates N times using it's output as next input.
// After N iterations, final k (12 digits) goes through sha256 to see if it 
// is below the target.


double multiply 
(double k) {
	 	double upper_digits = (int64_t)(k/1e6); 
	 	double lower_digits = (int64_t)(1e6*(k/1e6 - upper_digits));
		if (lower_digits == 0 || upper_digits == 0) { return k; }
		else {  return upper_digits*lower_digits; }

		// double lower_digits = modf (k/1e6, &intpart);
  	// double upper_digits = (int64_t)(1e6*intpart); 
}

double extract_12_digits 
	(double k) {
		int n; double intpart;
		// Get the significand from k in base 2
		k = frexp (k , &n);
		// Throw away top 2 significands. This has a
		// very important side effect: prevents the math operation
		// sequences from being simplified mathematicvally. 
		// If the result is 2 digits above double's smallest resolution
		// then it also prevents errors from differences in compiling doubles. 
		k = modf (100*k , &intpart); 
		k = (int64_t)(k*1e12);
		for (int i=1; i<14; i++) { 	if (k<1e11) { k*=10; }  }
		
  	return k;
} 

double select_and_do_operation
	(double k, int seq) {
		// The k passed to the function is modified to make keep k in a 
		// range that is valid and fast-changing for the function to 
		// reduce ability to optimize.
		
		if      ( seq == 1 )  { k = 1/k; }
		else if ( seq == 2 )  { k=(k*10)/1e12+.01; k = log(k); }
		else if ( seq == 3 )	{ k = sqrt(k);  }
		else if	( seq == 4 )	{ k=(10*k)/1e12; k = exp(k); }
		else if	( seq == 5 )	{ k = sin(k)+1.01; }
		else if	( seq == 6 )	{ k=k/1e12; k = asin(k); }
		else if	( seq == 7 )	{ k=(10*k)/1e12; k = sinh(k); }
		else if	( seq == 8 )	{ k=(10*k)/1e12; k = asinh(k); }
		else if	( seq == 9 )	{ k=(1.2*k)/1e12; k = erf(k); }
		else if	( seq == 10 )	{ k=k/1e12; k = tgamma(k); }
		else if	( seq == 11 ){ k = multiply(k); }
		// cout << seq << " " << k;
		k = extract_12_digits(k);
		// cout << " " << k << endl;
		return k;
}

int POW
 () {
int64_t iterations = 2000000; // for each nonce
int64_t nonce, nonces = 300; // number of nonces to simulate
double last_avg = 5e+11; // Assume avg was correct to get a std_dev
double hash = 1.234567801123456e76; // sha256 of (nonce + prev block hash)
double k, k2, intpart, fractpart, averg, std_dev;
int64_t sysTime;
int n, nFunctions, seq, function_count, temp;

// outer loop changes hash to simulate new nonce
for (nonce = 1; nonce < nonces; nonce++) {
	// Simulated nonce change. It will cause "hash" > 1E77,
	// but that should not affect the results.
	hash += 1.234567e75; 

	// Get the significand in base 2. Range is 0.5000 to 0.9999
	// k is significand & n => 2^n exponent
	n;
	k = frexp (hash , &n); 
	// The above did not give me a smooth "random" distribution but
	// the following seemed to fix it by throwing away top 2 digits.
	// Throwing away 1 digit was not enough to get correct average k.
	fractpart = modf (100*k , &intpart);

	// Get 12 digits of k as initial seed
	k = (int64_t)(fractpart*1e12);

	// First use of seed is to determine algorithm for this nonce.
	// I have 11 different math functions that will be performed twice.
	// This will give (11!)^2 possible algorithms.
	// Use modulo to get "random" function identifier 1 to 11 to be
	// stored in functions[] vector that is size 22.
	// Example: functions[0] will be the first of 22 functions. It's 
	// value is 0 to 10 which determins which function. 
	// Program will perform 0 to 21 math functions in sequence.
	nFunctions = 11;
	vector<int> functions(nFunctions*2);
	
	seq=0; function_count = 0; 	temp = 0;
	k2=k; // get copy of k
	int j=0;
	while ( j < 500 ) {	//seq < nFunctions*2
		j++;
		temp = 1+((int64_t)(k2) % nFunctions);
		// Seach functions found so far to make sure function has not 
		// been used twice before.
		for (int i=0; i < seq; i++) { 
				if (functions[i] == temp ) { function_count++;  } 
		}	
		// If function identifier has not been used twice, 
		// assign it to this sequence identifier. 
		if (function_count < 2) { functions[seq] = temp; seq++; }
		function_count=0;
		// Get next intpart using ... something that I hope works.
		k2 = sqrt(k2);
		k2 = extract_12_digits(k2);

	} // while all 22 seq slots are not filled

	cout << "Current algo's function sequence: " << endl; 
	for (int i=0; i<nFunctions; i++) { cout << functions[i] << " "; } 
	cout << endl;
	for (int i=nFunctions; i<nFunctions*2; i++) { 
		cout << functions[i] << " "; 
	} 	
	cout << endl;

	// get sysTime in milliseconds.
	sysTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	// ===== POW LOOP FOR THIS NONCE======. 
	// Repeat the algorithm many times, using each 
	// iteration's output as next input. 
	for (int64_t i = 1; i < iterations; i++) { 
		for (seq = 0; seq<nFunctions*2; seq++) {
			k = select_and_do_operation(k,functions[seq]); 
		}
		// get 2 metrics to check if output is random
		std_dev = pow(k-last_avg,2);
		averg = averg + k/iterations;
	}

	// Get metrics and solvetime for each algorithm's final value 
	std_dev = sqrt(std_dev);
	sysTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()  -  sysTime;

	cout << sysTime << "us, output k=" << k << endl;
	averg=0;
	std_dev=0;

} // next nonce

} // end POW

int main() 
{

POW();
return 0;

} ;

