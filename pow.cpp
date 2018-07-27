#include <iostream>
#include <vector>
#include <fstream> 
#include <string> 
#include <math.h>
#include <chrono>
#include <stdint.h>
#include <limits> // for being able to display all double digits
using namespace std; 

// Compiled with  g++ -std=c++8

// Copyright 2018 Zawy, MIT License
// This is a POW. It is not for actual use, but for demonstration.
// The goal is to have a simple, random, non-optimizable algorithm that 
// requires heavy processor use. The algorithm changes every nonce. 
// RAM or large cache should not make it faster, or my goal has 
// not been acheived. My belief is that ASICS, FPGAs, GPUs, CPUs, and 
// cell phones are on a more equal footing with this in terms of 
// hashes per Watt because the simpe math functions should already be 
// optimized, and this random combination of them should not be 
// mathematically optimizable.
//
// A key component is a "extract 12 digits routine.  It disconnects
// the previous function's math determininism from the next one
// so that a mathmatically or logicial simplification should not be possible.
// It also discards the lower 2 digits of the double to prevent differences
// in systems.
// 
// What it does:  
// k = sha265(nonce + previous hash)  // modeled but not included here.
// Use top 12 decimal digits of k to design algorithm and seed.
// Algorithm is a sequence of 8 possible math functions used exactly twice.
// Number of possible algorithms is a little over (8!)^2 = 1E9.
// Algorithm iterates N times using it's output as next input.
// After N iterations, final k (12 digits) goes through sha256 to see if it 
// is below the target.

long double extract_12_digits 
	(long double k) {
		int n; long double intpart;
		// Get the significand from k in base 2
		k = frexp (k , &n);
		// Throw away top 2 significands because of base 2 problem. This has a
		// very important side effect: prevents the math operation
		// sequences from being simplified mathematically. 
		k = modfl (100*k , &intpart); // modf for doubles, modfl for long doubles
		// Get remaining top 12 digits. Thows away lower digits that
		// might have had a difference on different systems.
		k = (int64_t)(k*1e12);
		// 		 for (int i=1; i<14; i++) { 	if (k<1e11) { k*=10; }  }
		// if (k < 1e11) { k*=10;}
		// else if (k < 1e10){ k*=100;}
		 if (k < 1e11)      { k *=10;  }
		 else if (k < 1e10) { k *=100;  }
		
  	return k;
} 

int POW
 () {
int64_t iterations = 100000; // for each nonce

int64_t nonce, nonces = 50; // number of nonces to simulate
vector<int64_t> output(nonces);
int nFunctions = 8;
int repeat = 2;
int nSeq = nFunctions*repeat;

long double last_avg = 5e+11; // Assume avg was correct to get a std_dev
long double hash = 1.234567801123456e76; // sha256 of (nonce + prev block hash)
long double k, k2, intpart, fractpart, averg, std_dev;
int64_t sysTime;
int n, seq, function_count, temp;

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
	fractpart = modfl (100*k , &intpart);  // modf for doubles, modfl for long doubles

	// Get 12 digits of k as initial seed
	k = (int64_t)(fractpart*1e12);

	// First use of seed is to determine algorithm for this nonce.
	// I have 8 different math functions that will be performed twice.
	// This will give (8!)^2 possible algorithms.
	// Use modulo to get "random" function identifier 1 to 8 to be
	// stored in functs[] vector that is size 16.
	// Example: functions[0] will be the first of 16 functs. It's 
	// value is 1 to 8 which determins which function. 
	// Program will perform 0 to 21 math functions in sequence.
	vector<int> functs(nSeq);
	
	seq=0; function_count = 0; 	temp = 0;
	k2=k; // get copy of k
	int j=0;
	while ( seq < nSeq ) {	//
		j++;
		temp = 1+((int64_t)(k2) % nFunctions);
		// Seach functions found so far to make sure function has not 
		// been used twice before.
		for (int i=0; i < seq; i++) { 
				if (functs[i] == temp ) { function_count++;  } 
		}	
		// If function identifier has not been used twice, 
		// assign it to this sequence identifier. 
		if (function_count < repeat) { functs[seq] = temp; seq++; }
		function_count=0;
		// Get psuedo random next intpart
		k2 *= k2;
		k2 = extract_12_digits(k2);

	} // while all 16 seq slots are not filled

		// Print output option
	if (1) {
		cout << "Current algo's function sequence: " << endl; 
		for (int i=0; i < nSeq; i++) { 
			cout << functs[i] << " "; 	
		} 	
		cout << endl;
	}

	 //  for (int i=0; i < nSeq; i++) {  functs[i] = 8; }

	// get sysTime in microseconds.
	sysTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

	// Test single fuinction:

	// ===== POW LOOP FOR THIS NONCE======. 
	// Repeat the algorithm many times, using each 
	// iteration's output as next input. 
	for (int64_t i = 1; i < iterations; i++) { 
			// multiply long doubles: 2.7 ns, int64_t = 1 ns
			// k=k; is 0 ns

		for (seq = 0; seq < nSeq; seq++) {

			// k = select_and_do_operation(k,functs[seq]); // old method

/*			// Switch was 1/2 as fast as "if" with equations, but 10x faster with k=k.
			switch (functs[seq]) {
				case 1: k = log(k);
				case 2: k = exp(k*1e-11);
				case 3: k = sin(k*6.28e-12)+1.01;
				case 4: k = pow(k,5.123);
				case 5: k = sinh(k*1e-11);
				case 6: k = asinh(k*1e-12);
				case 7: k = 1/erf(0.001*sqrt(sqrt(k)));  
				case 8: k = asin(k*1e-12);  
			}
*/		// if took about 2 ns per if.  But determining this was tricky.
			if ( functs[seq] == 1 ){ k = log(k); } // 80 ns, 100 ns w/ long doubles
			else if ( functs[seq] == 2 ){ k = pow(k,5.123); } // 60 ns
			else if ( functs[seq] == 3 ){ k = exp(k*1e-11); } // 72 ns
			else if ( functs[seq] == 4 ){ k = sin(k*6.28e-12)+1.01; } // 70 ns, 88 w/ long doubles
			else if ( functs[seq] == 5 ){ k = sinh(k*1e-11); } // 53 ns asinh(k*1e-12); 
			else if ( functs[seq] == 6 ){ k = asinh(k*1e-12);  } // 55 ns
			else if ( functs[seq] == 7 ){ k = 1/erf(0.001*sqrt(sqrt(k)));   } //  40 ns, 60 w/ longs
			else if ( functs[seq] == 8 ){ k = asin(k*1e-12); } // 30 ns, 50 w/ longs
			// the following is 16 ns with doubles, 33 ns long doubles.
			k = frexp (k , &n);
			k = modfl ( 100*k, &intpart); // modf for doubles, modfl for long doubles
			k = (int64_t)(k*1e12);  // this worked a lot better than trunc() which was unstable
		 	if (k < 1e11)      { k *=10;  }
			else if (k < 1e10) { k *=100;  } 
 
	} 
		// get 2 metrics to check if output is random
		// std_dev = pow(k-last_avg,2);
		// averg = averg + k/iterations;
	}
	cout << endl;

	// Get metrics and solvetime for each algorithm's final value 
	//std_dev = sqrt(std_dev);

	sysTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()  -  sysTime;

	 cout << sysTime << "us, output k=" << k << endl;
		
	averg=0;
	std_dev=0;
//	output[nonce] = sysTime;

} // next nonce

 /* ofstream file("pow_out.txt");
    for(int i=0; i<nonces; i++){
        file << output[i]  << endl;
    }
    file.close();
*/
} // end POW

int main() 
{

POW();

 // show all digits 
/*  std::cout << std::numeric_limits<long double>::digits10 << std::endl;
typedef std::numeric_limits< long double > ldbl;
long double d = 123456789012345678;
cout.precision(ldbl::max_digits10);
cout << "Pi: " << fixed << d << endl;  */
 
return 0;

} ;

