#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <thread>
#include <functional>
#include <sstream>
#include <map>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <cpuid.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#include <msr_core.h>
#include <msr_rapl.h>
#include <msr_counters.h>
#include <msr_rapl.h>
#include <libmsr_error.h>
#include <unistd.h>
#ifdef HASWELL
        #define MSR_CORE_FREQ 0x199
        #define MSR_CORE_FREQ_READ 0x188
        #define MAX_UNCORE_FREQ 30
        #define MIN_UNCORE_FREQ 12
#endif

#ifdef BROADWELL
	#define MSR_CORE_FREQ 0x199
	#define MSR_CORE_FREQ_READ 0x188
	#define MAX_UNCORE_FREQ 24
	#define MIN_UNCORE_FREQ 12
#endif

#ifdef SKYLAKE
	#define MSR_UNCORE_FREQ 0x620
	#define MSR_UNCORE_FREQ_READ 0x621
	#define MAX_UNCORE_FREQ 24
	#define MIN_UNCORE_FREQ 12
	#define UNCORE_FREQ_STEP 1
#endif

int main(int argc, char *argv[]) {
	unsigned i;

	uint64_t cores = 0;
	uint64_t threads = 0;
	uint64_t sockets = 0;
	core_config(&cores, &threads, &sockets, NULL);
	int freq=MAX_UNCORE_FREQ; 
	if(argc == 2) {
		freq = atoi( argv[1] );	
	}
	if(freq <MIN_UNCORE_FREQ || freq>MAX_UNCORE_FREQ) {
		printf("the given freq %d is invalid \n", freq);
		exit(-1);
	}
	//printf("freq to be set %d \n", freq);
	//printf("cores %d threads %d  sockets %d \n", cores, threads, sockets);

	if(init_msr()) {
		libmsr_error_handler("Unable to initialize libmsr", LIBMSR_ERROR_MSR_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
		exit(-1);
	}

	uint64_t tmp; 
	for(i=0; i<12; i++) {
		read_msr_by_coord(0, i, 0, MSR_CORE_FREQ, &tmp);
		printf("socket %d core %d freq %x \n", 0, i,tmp);
		//write_msr_by_coord(0, i, 0, MSR_CORE_FREQ, 0x1d00);
		//read_msr_by_coord(0, i, 0, MSR_CORE_FREQ_READ, &tmp);
		//printf("core freq %x \n", tmp);
	}
	for(i=0; i<12; i++) {
		read_msr_by_coord(1, i, 0, MSR_CORE_FREQ, &tmp);
		printf("socket %d core %d freq %x \n", 1, i,tmp);
		//write_msr_by_coord(1, i, 0, MSR_CORE_FREQ, 0x1d00);
		//read_msr_by_coord(1, i, 0, MSR_CORE_FREQ_READ, &tmp);
		//printf("core freq %x \n", tmp);
	}
	/*
	for(i=0; i<sockets; i++) {
		//the read values is accurate only if the processor is loaded with tasks
		read_msr_by_coord(0, i, 0, MSR_CORE_FREQ, &tmp);
		printf("S%d: core freq before %d \n", i, tmp); 
		//write_msr_by_coord(i, 0, 0, MSR_UNCORE_FREQ, freq);
		//read_msr_by_coord(i, 0, 0, MSR_UNCORE_FREQ_READ, &tmp);
		//printf("S%d: Uncore freq after %d \n", i, tmp); 
	}
	*/

	finalize_msr();
	return 0;
}
