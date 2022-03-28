#ifndef NODE_HPP
#define NODE_HPP
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <stdint.h>
#include "def.hpp"

class NODE{
	FILE * freq_fds[NUM_CORES];
	int coreState[NUM_CORES]; 
	int core_fds[NUM_CORES][NUM_CORE_EVENTS]; 
	int uncore_fds[NUM_SOCKETS][NUM_UNCORE_EVENTS];
	int numMetrics;

	long long corePerfValuesStart[NUM_CORES][NUM_CORE_EVENTS]; 
	long long corePerfValuesStop[NUM_CORES][NUM_CORE_EVENTS];

	long long uncorePerfValuesStart[NUM_SOCKETS][NUM_UNCORE_EVENTS]; 
	long long uncorePerfValuesStop[NUM_SOCKETS][NUM_UNCORE_EVENTS]; 

	uint64_t libmsrValuesStart[NUM_SOCKETS][NUM_LIBMSR_EVENTS];
	uint64_t libmsrValuesStop[NUM_SOCKETS][NUM_LIBMSR_EVENTS];

	void init_libmsr_collection();
	void start_libmsr_collection( int coreID );
	void stop_libmsr_collection(); 
	void finalize_libmsr_collection();

	void init_perf_collection();
	void start_perf_collection( int coreID );
	void stop_perf_core_collection(int coreID);
	void stop_perf_core_collection();
	void stop_perf_uncore_collection(); 
	void finalize_perf_collection();


	int perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags);
	void init_collection(); 
	void finalize_collection();
	
	private:
	int _calc_roof_latency_energy(float ins, float datavolume, float timeInterval, float timeCoreStall, float timeMemIdle, float fullBW, float coreFreq, float uncoreFreq, int flag, int *newCoreFreq, int * newUncoreFreq);
	int _calc_roof_latency_power(float ins, float datavolume, float timeInterval, float timeCoreStall, float timeMemIdle, float fullBW, float coreFreq, float uncoreFreq, int flag, int *newCoreFreq, int * newUncoreFreq);



	public:
	float insPerSec[NUM_SOCKETS]; 
	float pkgPower[NUM_SOCKETS];
	float dramPower[NUM_SOCKETS];
	float coreFreq[NUM_SOCKETS]; 
	float uncoreFreq[NUM_SOCKETS];
	int management_results[NUM_SOCKETS];

	//private:
	//int _calc_roof_latency_energy(float ins, float datavolume, float timeInterval, float timeCoreStall, float timeMemIdle, float fullBW, float coreFreq, float uncoreFreq, int flag, int *newCoreFreq, int * newUncoreFreq);
	
	NODE(); 
	~NODE();

	//called by/for each core
	void start_collection( ); 
	//called by/for each core
	void start_collection( int coreID );
	//called by/for each core
	void stop_core_collection( );
	//called by/for each core
	void stop_core_collection(int coreID);
	//void calc_roof(float timeInterval);
	void calc_roof(float timeInterval, int *newCoreFreq, int *newUncoreFreq);
	void stop_uncore_collection();

	void reset_node();
	void set_core_freq(int coreID, int freq);
	void set_socket_core_freq(int socketID, int freq);
	void set_node_core_freq(int freq);
	void set_socket_uncore_freq(int socketID, int freq);
	void set_node_uncore_freq(int freq);
};
#endif
