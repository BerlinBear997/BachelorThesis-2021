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
        #define MSR_UNCORE_FREQ 0x620
        #define MSR_UNCORE_FREQ_READ 0x621
        #define MAX_UNCORE_FREQ 30
        #define MIN_UNCORE_FREQ 12
        #define UNCORE_FREQ_STEP 1
#endif

#ifdef BROADWELL
	#define MSR_UNCORE_FREQ 0x620
	#define MSR_UNCORE_FREQ_READ 0x621
	#define MAX_UNCORE_FREQ 27
	#define MIN_UNCORE_FREQ 12
	#define UNCORE_FREQ_STEP 1
#endif

int main(int argc, char *argv[]) {
	unsigned i;

	uint64_t cores = 0;
	uint64_t threads = 0;
	uint64_t sockets = 0;
	core_config(&cores, &threads, &sockets, NULL);
	printf("cores %d threads %d  sockets %d \n", cores, threads, sockets);

	if(init_msr()) {
		libmsr_error_handler("Unable to initialize libmsr", LIBMSR_ERROR_MSR_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
		exit(-1);
	}

	for(i=0; i<sockets; i++) {
		uint64_t tmp; 
		//the read values is accurate only if the processor is loaded with tasks
		read_msr_by_coord(i, 0, 0, MSR_UNCORE_FREQ, &tmp);
		printf("S%d: limited uncore freq %d \n", i, tmp); 
		read_msr_by_coord(i, 0, 0, MSR_UNCORE_FREQ_READ, &tmp);
		printf("S%d: current uncore freq %d \n", i, tmp); 
	}

	finalize_msr();
	return 0;
}
