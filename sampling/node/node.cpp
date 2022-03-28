//set_socket_core_freq(i,newCoreFreq[i]);
                //set_socket_uncore_freq(i,newUncoreFreq[i]);#include <msr_core.h>
#include <msr_counters.h>
#include <sched.h>
#include <math.h>
#include "node.hpp"
#include "hw.hpp"

#define ROOF_THREASHOLD 0.05
#define POWER_FREQ_SHIFT_FACTOR 1.0

float PKGEnergy[NUM_SOCKETS]; 
float DRAMEnergy[NUM_SOCKETS];

struct read_format {
	uint64_t nr; 
	struct {
		uint64_t value;
		uint64_t id; 
  	} values[];
};

void NODE::init_libmsr_collection() {
	if(init_msr()) {
		printf("Error: Unable to initilize libmsr");
		exit(-1);
	}
#ifdef DEBUG
	printf("init freeze %x %x\n", 0x700, 0x80000000);
#endif
	write_msr_by_coord(START_SOCKET, 0, 0, 0x700, 0x80000000);
	int i, j; 
	for(j=START_SOCKET; j<END_SOCKET; j++) {
		for(i=0; i<NUM_LIBMSR_CTL_EVENTS; i++) {
			write_msr_by_coord(START_SOCKET, 0, 0, libmsr_control_events[i].idx, libmsr_control_events[i].enable);
		}
	}
	//printf("\n\n");
	for(j=START_SOCKET; j<END_SOCKET; j++) {
		for(i=0; i<NUM_LIBMSR_EVENTS; i++) {
			write_msr_by_coord(START_SOCKET, 0, 0, libmsr_events[i].s_idx, libmsr_events[i].config);
			write_msr_by_coord(START_SOCKET, 0, 0, libmsr_events[i].v_idx, 0x0);
		}
	}
	//unfreeze
#ifdef DEBUG
	printf("init unfreeze %x %x\n", 0x700, 0x20000000);
#endif
	write_msr_by_coord(START_SOCKET, 0, 0, 0x700, 0x20000000);
}

void NODE::start_libmsr_collection(int coreID) {
	int i,j;
	//freeze uncore
	//write_msr_by_coord(START_SOCKET, 0, 0, 0x700, 0x80000000);
	if(coreID % NUM_CORES_PER_SOCKET == 0) {
		int socketID = coreID / NUM_CORES_PER_SOCKET;
		for(i=0; i<NUM_LIBMSR_EVENTS; i++) {
			read_msr_by_coord(socketID,0,0, libmsr_events[i].v_idx, &libmsrValuesStart[socketID][i]);
		}
	}
	//unfreeze uncore
	//write_msr_by_coord(START_SOCKET, 0, 0, 0x700, 0x20000000);
	return;
}

void NODE::stop_libmsr_collection() {
	int i, j; 

	for(j=START_SOCKET; j<END_SOCKET; j++) {
		for(i=0; i<NUM_LIBMSR_EVENTS; i++) {
			read_msr_by_coord(j,0,0, libmsr_events[i].v_idx, &libmsrValuesStop[j][i]);
		}
	}

	return;
}

void NODE::finalize_libmsr_collection() {
	//freeze collection
	write_msr_by_coord(START_SOCKET, 0, 0, 0x700, 0x80000000);
	int i, j; 
	for(j=0; j<NUM_SOCKETS; j++) {
		for(i=0; i<NUM_LIBMSR_CTL_EVENTS; i++) {
			write_msr_by_coord(START_SOCKET, 0, 0, libmsr_control_events[i].idx, libmsr_control_events[i].disable);
		}
	}
	for(j=0; j<NUM_SOCKETS; j++) {
		for(i=0; i<NUM_LIBMSR_EVENTS; i++) {
			write_msr_by_coord(START_SOCKET, 0, 0, libmsr_events[i].s_idx, 0x0);
			write_msr_by_coord(START_SOCKET, 0, 0, libmsr_events[i].v_idx, 0x0);
		}
	}
	write_msr_by_coord(START_SOCKET, 0, 0, 0x700, 0x0);
}
	

int NODE::perf_event_open(struct perf_event_attr *hw_event, pid_t pid, int cpu, int group_fd, unsigned long flags){
	int ret;
	ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
			group_fd, flags);
		return ret;
}

void NODE::init_perf_collection() {
	struct perf_event_attr pe, upe;
	int fd_leader;

	memset(&pe, 0, sizeof(struct perf_event_attr));	
	pe.size = sizeof(struct perf_event_attr);
	pe.disabled = 1;
	pe.read_format=PERF_FORMAT_GROUP | PERF_FORMAT_ID;
	//pe.exclude_kernel = 1;
	//pe.exclude_hv = 1;
	int i,j; 
	printf("Start_core %d End_core %d  num_core_events %d \n", START_CORE, END_CORE, NUM_CORE_EVENTS);
	for(i=START_CORE; i<END_CORE; i++) {
		for(j=0; j<NUM_CORE_EVENTS; j++) {
			pe.type = core_events[j].type;
			pe.config = core_events[j].config;

			//if( (i+j) == 0){
			if( j == 0){
				core_fds[i][j] = perf_event_open(&pe, -1, i , -1, 0);
				fd_leader = core_fds[i][j];
			} else {
				//core_fds[i][j] = perf_event_open(&pe, -1, i, -1, 0);
				core_fds[i][j] = perf_event_open(&pe, -1, i, fd_leader, 0);
			}

			if (core_fds[i][j] == -1) {
				fprintf(stderr, "Error opening core event: core %d event %d type %d config %x: %s \n", i, j, pe.type, pe.config, strerror(core_fds[i][j]));
				exit(EXIT_FAILURE);
			}
		}
	}

	memset(&upe, 0, sizeof(struct perf_event_attr));	
	upe.size = sizeof(struct perf_event_attr);
	upe.disabled = 1;
	for(i=START_SOCKET; i<END_SOCKET; i++) {
		for(j=0; j<NUM_UNCORE_EVENTS; j++) {
			upe.type = uncore_events[j].type; 
			upe.config = uncore_events[j].config; 
			uncore_fds[i][j] = perf_event_open(&upe, -1, i*NUM_CORES_PER_SOCKET, -1, 0);

			if (uncore_fds[i][j] == -1) {
				fprintf(stderr, "Error opening uncore event: core %d event %d type %d config %x: %s \n", i, j, upe.type, upe.config, strerror(uncore_fds[i][j]));
				exit(EXIT_FAILURE);
			}
		}
	}

	//enable perf core measurement
	for(i=START_CORE; i<END_CORE; i++) {
		j=0;
		//for(j=0; j<NUM_CORE_EVENTS; j++) {
			ioctl(core_fds[i][j], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
			ioctl(core_fds[i][j], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
		//}
	}
	//enable perf uncore measuremnt
	for(i=START_SOCKET; i<END_SOCKET; i++) {
		for(j=0; j<NUM_UNCORE_EVENTS; j++) {
			ioctl(uncore_fds[i][j], PERF_EVENT_IOC_RESET, 0);
			ioctl(uncore_fds[i][j], PERF_EVENT_IOC_ENABLE, 0);
		}
	}

}

void NODE::init_collection() {
	init_perf_collection(); 
	init_libmsr_collection();

	//set_socket_core_freq(START_SOCKET, CORE_MAX_FREQ);
	//set_socket_uncore_freq(START_SOCKET, UNCORE_MAX_FREQ);
}

void NODE::start_perf_collection(int coreID) {
	int i,j;
	char buf[4096];
	struct read_format * rf = (struct read_format*) buf; 
	//ioctl(core_fds[i][0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
	read(core_fds[coreID][0], buf, sizeof(buf));
	for (j=0; j<rf->nr; j++) {
		corePerfValuesStart[coreID][j] = rf->values[j].value;
	}
	//ioctl(core_fds[i][0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);

	if(coreID %  NUM_CORES_PER_SOCKET == 0) {
		int socketID = coreID / NUM_CORES_PER_SOCKET;
		for(i=0; i<NUM_UNCORE_EVENTS; i++){
			//long long t;
			//read(uncore_fds[j][i], &t, sizeof(long long));
			read(uncore_fds[socketID][i], &uncorePerfValuesStart[socketID][i], sizeof(long long));	
		}
	} 
}

void NODE::start_collection() {
	int coreID = sched_getcpu();
	start_collection( coreID );
	start_perf_collection(coreID);
}

void NODE::start_collection(int coreID) {
	start_libmsr_collection( coreID ); 
	start_perf_collection( coreID );
}

void NODE::stop_perf_core_collection() {
	int coreID = sched_getcpu();
	stop_perf_core_collection( coreID );
}

//each core stops its own core value
void NODE::stop_perf_core_collection(int coreID) {
	char buf[4096];
	int i;
	struct read_format * rf = (struct read_format*) buf; 
	//ioctl(core_fds[coreID][0], PERF_EVENT_IOC_DISABLE, PERF_IOC_FLAG_GROUP);
	read(core_fds[coreID][0], buf, sizeof(buf));
	for (i=0; i<rf->nr; i++) {
		corePerfValuesStop[coreID][i] = rf->values[i].value;
	}
	//ioctl(core_fds[coreID][0], PERF_EVENT_IOC_ENABLE, PERF_IOC_FLAG_GROUP);
	//read(core_fds[coreID][i], &corePerfValuesStop[coreID][i], sizeof(long long));
}

void NODE::stop_core_collection(int coreID) {
	stop_perf_core_collection( coreID );
}

void NODE::stop_core_collection( ) {
	int coreID = sched_getcpu(); 
	stop_perf_core_collection( coreID );
}

void NODE::stop_perf_uncore_collection() {
	int i, j;
	for(j=START_SOCKET; j<END_SOCKET; j++) {
		for(i=0; i<NUM_UNCORE_EVENTS; i++){
			read(uncore_fds[j][i], &uncorePerfValuesStop[j][i], sizeof(long long));	
		}
	}
}

void NODE::stop_uncore_collection() {
	stop_perf_uncore_collection();	
	stop_libmsr_collection();
}


void NODE::calc_roof(float timeInterval, int *newCoreFreq, int *newUncoreFreq) {
	int i, j;
	stop_uncore_collection();

	for(i=START_SOCKET; i<END_SOCKET; i++) {
#ifdef DEBUG
		printf("Socket : %d\n", i);
		printf("WallTime : %f \n", timeInterval );
#endif
		//printf("id 12 %llu %llu \n", libmsrValuesStop[i][12], libmsrValuesStart[i][12]);
		uncoreFreq[i] = 0;
		uncoreFreq[i] += libmsrValuesStop[i][18] - libmsrValuesStart[i][18];
		uncoreFreq[i] = uncoreFreq[i]/timeInterval;
#ifdef DEBUG
		printf("Uncorefreq: %.4e \n", uncoreFreq[i]);
#endif

		int startCore = i*NUM_CORES_PER_SOCKET; 
		int endCore = (i+1) * NUM_CORES_PER_SOCKET;
		float ins=0, cyc=0, refCyc=0; 
		for(j=startCore; j<endCore; j++) {
			ins += corePerfValuesStop[j][0] - corePerfValuesStart[j][0];
			cyc += corePerfValuesStop[j][1] - corePerfValuesStart[j][1];
			refCyc += corePerfValuesStop[j][2] - corePerfValuesStart[j][2];
		}

		coreFreq[i] = cyc/refCyc * CLOCK; 
                printf("CoreFreq : %.4e \n", coreFreq[i]);

		float inverseFreq = 1.0 / coreFreq[i];
		float inverseCores = 1.0 / (float) NUM_CORES_PER_SOCKET;
		float clockTime = cyc * inverseFreq * inverseCores;
#ifdef DEBUG
		printf("Core unhlated time : %f \n", clockTime );
                printf("Core halted time : %f \n", timeInterval - clockTime);
                printf("--------\n");
#endif

		float stallL2=0, stallLDM=0, stallAll=0; 
		float pendL2 = 0, pendLDM = 0;
		//float l3a=0, l3m=0;
		//float stallAll = 0;
		for(j=startCore; j<endCore; j++) {
			stallL2 += corePerfValuesStop[j][3] - corePerfValuesStart[j][3];
			stallLDM += corePerfValuesStop[j][4] - corePerfValuesStart[j][4];
			stallAll += float(corePerfValuesStop[j][5] - corePerfValuesStart[j][5]);
		}
		float timeStallAll = stallAll * inverseFreq * inverseCores;
                float timeStallL2 = stallL2 * inverseFreq * inverseCores;
                float timeStallLDM = stallLDM * inverseFreq * inverseCores;

                float timeHaltedStallAll = timeStallAll + timeInterval - clockTime;
                float timeHaltedStallL2 = timeStallL2 + timeInterval - clockTime;
                float timeHaltedStallLDM = timeStallLDM + timeInterval - clockTime;

#ifdef DEBUG
		printf("ins cumulative: %f \n", ins);
                printf("cycles cumulative: %f \n", cyc);
                printf("L2 stalld : %f \n ", stallL2);
                printf("Ldm stalld : %f \n", stallLDM);
                printf("total stalld : %f  \n", stallAll);
                printf("cycles : %f  \n", cyc);
                printf("--------\n");
                printf("Time: Core : running %f, total stall: %f, stall+halted: %f\n",  timeInterval-timeHaltedStallAll, timeStallAll, timeHaltedStallAll);
                printf("Time: Core : running %f, l2 stall: %f, stall+halted: %f\n",  timeInterval-timeHaltedStallL2, timeStallL2, timeHaltedStallL2);
                printf("Time: Core : running %f, ldm stall: %f, stall+halted: %f\n",  timeInterval-timeHaltedStallLDM, timeStallLDM, timeHaltedStallLDM);
		printf("Rate: Core : running %f%, total stall: %f%, stall+halted: %f%\n", (timeInterval-timeHaltedStallAll)*100/timeInterval, timeStallAll*100/timeInterval, timeHaltedStallAll*100/timeInterval);
		printf("Rate: Core : running %f%, l2 stall: %f%, stall+halted: %f%\n",  (timeInterval-timeHaltedStallL2)*100/timeInterval, timeStallL2*100/timeInterval, timeHaltedStallL2*100/timeInterval);
		printf("Rate: Core : running %f%, ldm stall: %f%, stall+halted: %f%\n",  (timeInterval-timeHaltedStallLDM)*100/timeInterval, timeStallLDM*100/timeInterval, timeHaltedStallLDM*100/timeInterval);

#endif
		float frac = (uncoreFreq[i] - int(uncoreFreq[i] * 1.0E-8) * 1.0E8) / 1.0E8;
                int lowerBound = int(uncoreFreq[i] * 1.0E-8) - 12;
                int upperBound = lowerBound + 1;
                int numEntry=sizeof(FULL_RAM_BW)/sizeof(float);
                float fullL3BW = (FULL_L3_BW[upperBound] - FULL_L3_BW[lowerBound]) * frac + FULL_L3_BW[lowerBound];
                float fullRAMBW = (FULL_RAM_BW[upperBound] - FULL_RAM_BW[lowerBound]) * frac + FULL_RAM_BW[lowerBound];

		//measured RAM read/write bandwidth
		float ramRead=0, ramWrite=0;
		ramRead += uncorePerfValuesStop[i][0] - uncorePerfValuesStart[i][0];
		ramRead += uncorePerfValuesStop[i][2] - uncorePerfValuesStart[i][2];
		ramRead += uncorePerfValuesStop[i][4] - uncorePerfValuesStart[i][4];
		ramRead += uncorePerfValuesStop[i][6] - uncorePerfValuesStart[i][6];
		ramWrite += uncorePerfValuesStop[i][1] - uncorePerfValuesStart[i][1];
		ramWrite += uncorePerfValuesStop[i][3] - uncorePerfValuesStart[i][3];
		ramWrite += uncorePerfValuesStop[i][5] - uncorePerfValuesStart[i][5];
		ramWrite += uncorePerfValuesStop[i][7] - uncorePerfValuesStart[i][7];
		ramRead = ramRead  * 64.0; 
		ramWrite = ramWrite  * 64.0; 
		float ram = ramRead + ramWrite;
		float memBusy = ram/fullRAMBW;
                float pkgEnergy = 0, dramEnergy =0;

                pkgEnergy +=  uncorePerfValuesStop[i][8] - uncorePerfValuesStart[i][8];
                dramEnergy +=  uncorePerfValuesStop[i][9] - uncorePerfValuesStart[i][9];
                pkgEnergy = pkgEnergy * 0.00000000023283064365386962890625;
                dramEnergy = dramEnergy * 0.00000000023283064365386962890625;

                pkgPower[i] = pkgEnergy / timeInterval;
                dramPower[i] = dramEnergy / timeInterval;

		//measured L3 bandwidth
		float l3=0;
		l3 += libmsrValuesStop[i][0] - libmsrValuesStart[i][0]; 
		l3 += libmsrValuesStop[i][1] - libmsrValuesStart[i][1]; 
		l3 += libmsrValuesStop[i][2] - libmsrValuesStart[i][2]; 
		l3 += libmsrValuesStop[i][3] - libmsrValuesStart[i][3]; 
		l3 += libmsrValuesStop[i][4] - libmsrValuesStart[i][4]; 
		l3 += libmsrValuesStop[i][5] - libmsrValuesStart[i][5]; 
		l3 += libmsrValuesStop[i][6] - libmsrValuesStart[i][6]; 
		l3 += libmsrValuesStop[i][7] - libmsrValuesStart[i][7]; 
		l3 += libmsrValuesStop[i][8] - libmsrValuesStart[i][8]; 
		l3 += libmsrValuesStop[i][9] - libmsrValuesStart[i][9]; 
		l3 += libmsrValuesStop[i][10] - libmsrValuesStart[i][10]; 
		l3 += libmsrValuesStop[i][11] - libmsrValuesStart[i][11];
		l3 += libmsrValuesStop[i][12] - libmsrValuesStart[i][12];
		l3 += libmsrValuesStop[i][13] - libmsrValuesStart[i][13];
		l3 += libmsrValuesStop[i][14] - libmsrValuesStart[i][14];
		l3 += libmsrValuesStop[i][15] - libmsrValuesStart[i][15];
		l3 += libmsrValuesStop[i][16] - libmsrValuesStart[i][16];
		l3 += libmsrValuesStop[i][17] - libmsrValuesStart[i][17]; 
		l3 = l3 * 64.0;

		//float l3Busy = l3/2.24E11;
                float l3Busy = l3/fullL3BW;
                if(upperBound >= numEntry){
                        upperBound = numEntry- 1;
                }

		float memBusyRate = memBusy*100/timeInterval < 100 ? memBusy*100/timeInterval : 100;
		float l3BusyRate = l3Busy*100/timeInterval < 100 ? l3Busy*100/timeInterval : 100;
		float memIdleRate = 100- memBusyRate;
		float l3IdleRate = 100 - l3BusyRate;
		
		float memIdle = timeInterval - memBusy < 0 ? 0 : timeInterval - memBusy;
		float l3Idle =  timeInterval - l3Busy < 0 ? 0 :  timeInterval - l3Busy; 

		
#ifdef DEBUG
                printf("Time: Mem Busy: %f idle %f\n", memBusy, memIdle);
                printf("Time: L3 busy: %f idle %f\n", l3Busy, l3Idle);
		printf("Rate: Mem Busy: %f%, idle %f%\n", memBusyRate, memIdleRate);
                printf("Rate: L3 busy: %f%, idle %f%\n", l3BusyRate, l3IdleRate);
		printf("L3 volum: %.4e\n", l3);
                printf("RAM volum: %.4e\n", ram);
                printf("lowerBound : %d \n upperBound : %d\n", lowerBound, upperBound);
                printf("full L3 BW : %.4e\n fullRAMBW: %.4e\n", fullL3BW, fullRAMBW);
                printf("PKG Energy: %f \n PKG Power : %f \n", pkgEnergy, pkgEnergy/timeInterval);
                printf("DRAM Energy: %f \n DRAM Power : %f \n", dramEnergy, dramEnergy/timeInterval);
#endif
                printf("***** Roofline L3 ******\n");
		int newCoreFreqL3, newUncoreFreqL3;
		int l3Boundness, ramBoundness;
		int newCoreFreqRAM, newUncoreFreqRAM;
#ifdef ENERGY
		l3Boundness = _calc_roof_latency_energy( ins, l3, timeInterval, timeHaltedStallL2, l3Idle, fullL3BW, coreFreq[i], uncoreFreq[i], 0, &newCoreFreqL3, &newUncoreFreqL3);
                printf("***** Roofline DRAM ******\n");
		//int newCoreFreqRAM, newUncoreFreqRAM;
                ramBoundness = _calc_roof_latency_energy( ins, ram, timeInterval, timeHaltedStallLDM, memIdle, fullRAMBW, coreFreq[i], uncoreFreq[i], 1, &newCoreFreqRAM, &newUncoreFreqRAM);

#endif 
#ifdef POWERCAP
		l3Boundness = _calc_roof_latency_power( ins, l3, timeInterval, timeHaltedStallL2, l3Idle, fullL3BW, coreFreq[i], uncoreFreq[i], 0, &newCoreFreqL3, &newUncoreFreqL3);
                printf("***** Roofline DRAM ******\n");
                ramBoundness = _calc_roof_latency_power( ins, ram, timeInterval, timeHaltedStallLDM, memIdle, fullRAMBW, coreFreq[i], uncoreFreq[i], 1, &newCoreFreqRAM, &newUncoreFreqRAM);
#endif
                printf("***** Conclusion ******\n");
		printf("l3BOundness %d dramBoundness %d  \n", l3Boundness, ramBoundness);
		//if ( newUncoreFreqL3 == UNCORE_MAX_FREQ && newUncoreFreqRAM == UNCORE_MAX_FREQ) {
		if( l3Boundness == -1 ) {
			if (ramBoundness == -1) {
				newUncoreFreq[i] = UNCORE_MAX_FREQ; 
				newCoreFreq[i] = newCoreFreqRAM > newCoreFreqL3? newCoreFreqRAM : newCoreFreqL3;
			} else {
				newUncoreFreq[i] = UNCORE_MAX_FREQ; 
				newCoreFreq[i] = newCoreFreqL3;
			}
			//set_socket_core_freq(i,newCoreFreq[i]);
                        //set_socket_uncore_freq(i,newUncoreFreq[i]);
		} else if (l3Boundness == 1) {
			if (ramBoundness == -1) {
				newUncoreFreq[i] = UNCORE_MAX_FREQ;
				newCoreFreq[i] = newCoreFreqRAM; 
				//set_socket_core_freq(i,newCoreFreq[i]);
                                //set_socket_uncore_freq(i,newUncoreFreq[i]);
			} else if (ramBoundness == 1)  {
				newCoreFreq[i] = CORE_MAX_FREQ;
				newUncoreFreq[i] = newUncoreFreqRAM > newUncoreFreqL3? newUncoreFreqRAM : newUncoreFreqL3; 
				//set_socket_core_freq(i,newCoreFreq[i]);
                                //set_socket_uncore_freq(i,newUncoreFreq[i]);
			} else {
				//let the setting as it is
			}
		} else { 
			if (ramBoundness == -1) {
				newUncoreFreq[i] = UNCORE_MAX_FREQ;
				newCoreFreq[i] = newCoreFreqRAM;
				//set_socket_core_freq(i,newCoreFreq[i]);
                                //set_socket_uncore_freq(i,newUncoreFreq[i]); 
			}  else {
				//let the setting as it is
				//set_socket_core_freq(i,newCoreFreq[i]);
                		//set_socket_uncore_freq(i,newUncoreFreq[i]);
			}
		}
		printf("new setting : core %d  uncore %d \n", newCoreFreq[i], newUncoreFreq[i]);
		set_socket_core_freq(i,newCoreFreq[i]);
		set_socket_uncore_freq(i,newUncoreFreq[i]);
        }
	
}

//flag = 0: for l3 cache
//flag = 1: for mem
//return value : -1 memory bound, 1 compute bound, 0 balanced
int NODE::_calc_roof_latency_power( float ins, float dataVolume, float timeInterval, float timeCoreStall, float timeMemIdle, float fullBW, float coreFreq, float uncoreFreq, int flag, int * newCoreFreq, int * newUncoreFreq){
	// the measured memory bandwidth can be higher than the theoretical peak
        if (timeMemIdle < 0 )
                timeMemIdle = 0;

        //the execution time can be brocken into time where roofline can be employed, 
        //and time which is bound to memory access latency.
        float timeLatency = timeCoreStall < timeMemIdle? timeCoreStall : timeMemIdle;
        float timeRoof = timeInterval - timeLatency;
	int boundness = 0;

        int i_coreFreq = (int)round( coreFreq * 1.0E-8 );
        int i_uncoreFreq = (int)round( uncoreFreq * 1.0E-8 );
#ifdef DEBUG
        printf("ins %f, datavolume %f, timeinterval %f, timeCoreStall %f, timeMemIdle %f, fullBW %f, coreFreq %f, uncoreFreq %f \n", ins, dataVolume, timeInterval, timeCoreStall, timeMemIdle, fullBW, coreFreq, uncoreFreq);
        printf("timeRoof %f timeLatency %f \n", timeRoof, timeLatency);
        printf("coreFreq %d uncoreFreq %d \n", i_coreFreq, i_uncoreFreq);
#endif

        float timeCoreBusy = timeInterval - timeCoreStall;
        float timeMemBusy = timeInterval - timeMemIdle;

#ifdef ROOFLINE
        //the measured operatinoal intensity
        float realIntensity = ins / dataVolume;

        float idealInsPerSec = ins / (timeInterval - timeCoreStall );
        float roofIntensity = idealInsPerSec / fullBW;
        if ( realIntensity < roofIntensity * ( 1 - ROOF_THREASHOLD) ) {
#else
        if ( timeCoreBusy < timeMemBusy * (1 - ROOF_THREASHOLD) ) {
#endif
		//memory bound
		boundness = -1;
		(*newUncoreFreq) = UNCORE_MAX_FREQ;
		
		//there is no freq between turbo and normal max freq
		if (i_coreFreq > CORE_MAX_FREQ_NO_TURBO) {
			(*newCoreFreq) = CORE_MAX_FREQ_NO_TURBO;
		} else {
			(*newCoreFreq) = i_coreFreq - CORE_STEP_FREQ; 
		}
		if ( (*newCoreFreq) < CORE_MIN_FREQ ) {
			(*newCoreFreq ) = CORE_MIN_FREQ;
		}

		float slowingFactor = (float) i_coreFreq / (float) (*newCoreFreq);
		float estimateNewUncoreFreq = uncoreFreq + UNCORE_STEP_FREQ * 1.0E8 * POWER_FREQ_SHIFT_FACTOR; 
		float accelerateFactor = uncoreFreq / estimateNewUncoreFreq;

		float newTimeCoreBusy = slowingFactor * timeCoreBusy; 
		float newTimeMemBusy = accelerateFactor * timeMemBusy;
		float newTimeLatency = slowingFactor * timeLatency;
		float newTimeInterval = newTimeMemBusy + newTimeLatency;

#ifdef DEBUG
		printf("timeInterval %f  timeCoreBusy %f timeMemBusy %f timeLatency %f\n", timeInterval, timeCoreBusy, timeMemBusy, timeLatency);
		printf("newTimeInterval %f  newTimeCoreBusy %f newTimeMemBusy %f newTimeLatency %f\n", newTimeInterval, newTimeCoreBusy, newTimeMemBusy, newTimeLatency);
#endif
		if( newTimeCoreBusy > newTimeMemBusy || newTimeInterval > timeInterval ) {
			(*newCoreFreq) += CORE_STEP_FREQ;
			if (i_coreFreq > CORE_MAX_FREQ_NO_TURBO) {
				(*newCoreFreq) = CORE_MAX_FREQ;
			}
		}
		
#ifdef DEBUG
		printf("core %d  uncore %d \n", (*newCoreFreq), (*newUncoreFreq));
		printf("memory bound new Freq core %d uncore %d \n\n", (*newCoreFreq), (*newUncoreFreq));
#endif
		return boundness;
#ifdef ROOFLINE
        } else if ( realIntensity > roofIntensity * ( 1 + ROOF_THREASHOLD ) ) {
#else
        } else if ( timeCoreBusy > timeMemBusy * (1+ROOF_THREASHOLD) ) {
#endif
		boundness = 1;
		(*newCoreFreq) = CORE_MAX_FREQ;
		(*newUncoreFreq ) = i_uncoreFreq - CORE_STEP_FREQ;
		if ( (*newUncoreFreq) < UNCORE_MIN_FREQ) {
			(*newUncoreFreq) = UNCORE_MIN_FREQ;
		}

		float slowingFactor = (float) i_uncoreFreq / (float) (*newUncoreFreq);
		float estimateNewCoreFreq = coreFreq + CORE_STEP_FREQ * 1.0E8 * POWER_FREQ_SHIFT_FACTOR; 
		float accelerateFactor = uncoreFreq / estimateNewCoreFreq;

		float newTimeMemBusy = slowingFactor * timeMemBusy; 
		float newTimeCoreBusy = accelerateFactor * timeCoreBusy;
		float newTimeLatency = slowingFactor * timeLatency;
		float newTimeInterval = newTimeCoreBusy + newTimeLatency;

#ifdef DEBUG
		printf("timeInterval %f  timeCoreBusy %f timeMemBusy %f timeLatency %f\n", timeInterval, timeCoreBusy, timeMemBusy, timeLatency);
		printf("newTimeInterval %f  newTimeCoreBusy %f newTimeMemBusy %f newTimeLatency %f\n", newTimeInterval, newTimeCoreBusy, newTimeMemBusy, newTimeLatency);
#endif
		if( newTimeMemBusy > newTimeCoreBusy || newTimeInterval > timeInterval ) {
			(*newUncoreFreq) += CORE_STEP_FREQ; 
		}

#ifdef DEBUG
		printf("core %d  uncore %d \n", (*newCoreFreq), (*newUncoreFreq));
		printf("compute bound new Freq core %d uncore %d \n\n", (*newCoreFreq), (*newUncoreFreq));
#endif
		return boundness;
        } else {
		return boundness;
#ifdef DEBUG
		printf("balanced new Freq core %d uncore %d \n\n", (*newCoreFreq), (*newUncoreFreq));
#endif
        }
}

//flag = 0: for l3 cache
//flag = 1: for mem
//return value : -1 memory bound, 1 compute bound, 0 balanced
int NODE::_calc_roof_latency_energy( float ins, float dataVolume, float timeInterval, float timeCoreStall, float timeMemIdle, float fullBW, float coreFreq, float uncoreFreq, int flag, int * newCoreFreq, int * newUncoreFreq){
	// the measured memory bandwidth can be higher than the theoretical peak
        if (timeMemIdle < 0 )
                timeMemIdle = 0;

        //the execution time can be brocken into time where roofline can be employed, 
        //and time which is bound to memory access latency.
        float timeLatency = timeCoreStall < timeMemIdle? timeCoreStall : timeMemIdle;
        float timeRoof = timeInterval - timeLatency;
	int boundness = 0;

        int i_coreFreq = (int)round( coreFreq * 1.0E-8 );
        int i_uncoreFreq = (int)round( uncoreFreq * 1.0E-8 );
#ifdef DEBUG
        printf("ins %f, datavolume %f, timeinterval %f, timeCoreStall %f, timeMemIdle %f, fullBW %f, coreFreq %f, uncoreFreq %f \n", ins, dataVolume, timeInterval, timeCoreStall, timeMemIdle, fullBW, coreFreq, uncoreFreq);
        printf("timeRoof %f timeLatency %f \n", timeRoof, timeLatency);
        printf("coreFreq %d uncoreFreq %d \n", i_coreFreq, i_uncoreFreq);
#endif

        float timeCoreBusy = timeInterval - timeCoreStall;
        float timeMemBusy = timeInterval - timeMemIdle;

#ifdef ROOFLINE
        //the measured operatinoal intensity
        float realIntensity = ins / dataVolume;

        float idealInsPerSec = ins / (timeInterval - timeCoreStall );
        float roofIntensity = idealInsPerSec / fullBW;
        if ( realIntensity < roofIntensity * ( 1 - ROOF_THREASHOLD) ) {
#else
        if ( timeCoreBusy < timeMemBusy * (1 - ROOF_THREASHOLD) ) {
#endif

		//memory bound
		boundness = -1;
		(*newUncoreFreq) = UNCORE_MAX_FREQ;
		
		//calculate exeuction time if the core is at maxisum
		float timeAtMaxFreq = timeLatency * (float) i_coreFreq / (float) CORE_MAX_FREQ + timeMemBusy;
#ifdef DEBUG
		printf("memory bound time atMaxFreq %f latency %f coreBusy %f factor %f\n", timeAtMaxFreq, timeLatency, timeCoreBusy, (float) i_coreFreq / (float) CORE_MAX_FREQ  );
#endif
		if (timeAtMaxFreq * (1 + ROOF_THREASHOLD) < timeInterval  ||  uncoreFreq < 0.99 * (float) UNCORE_MAX_FREQ * 1.0E8 ) {
#ifdef DEBUG
			printf("Oversterring, reset core by one uncoreFreq %f timeAtMaxFreq %f timeInterval %f\n", uncoreFreq, timeAtMaxFreq, timeInterval );
#endif
			(*newCoreFreq) = i_coreFreq + CORE_STEP_FREQ;
			if (i_coreFreq > CORE_MAX_FREQ_NO_TURBO) {
				(*newCoreFreq) = CORE_MAX_FREQ;
			}
			return boundness;
		}

		//there is no freq between turbo and normal max freq
		if (i_coreFreq > CORE_MAX_FREQ_NO_TURBO) {
			(*newCoreFreq) = CORE_MAX_FREQ_NO_TURBO;
		} else {
			(*newCoreFreq) = i_coreFreq - CORE_STEP_FREQ; 
		}
		if ( (*newCoreFreq) < CORE_MIN_FREQ ) {
			(*newCoreFreq ) = CORE_MIN_FREQ;
		}

		float factor = (float) i_coreFreq / (float) (*newCoreFreq);

		float newTimeLatency = timeLatency * factor; 
		float newTimeInterval = newTimeLatency + timeMemBusy;
#ifdef DEBUG
		printf("timeInterval %f  timeCoreBusy %f timeMemBusy %f 1.05*timeAtMaxFreq %f new time latency %f , new timeInterval %f noBottlenecktime %f\n", timeInterval, timeCoreBusy, timeMemBusy, 1.05*timeAtMaxFreq, newTimeLatency, newTimeInterval, timeCoreBusy * factor );
		printf(" latency procent %f \n", newTimeLatency / newTimeInterval * 100);
#endif
		if ( (timeCoreBusy * factor > timeMemBusy) || (newTimeInterval > (1+ROOF_THREASHOLD) * timeAtMaxFreq) || (newTimeLatency > ROOF_THREASHOLD * newTimeInterval)) {
			(*newCoreFreq) += CORE_STEP_FREQ;
			if (i_coreFreq > CORE_MAX_FREQ_NO_TURBO) {
				(*newCoreFreq) = CORE_MAX_FREQ;
			}
			//return boundness;
		}
#ifdef DEBUG
		printf("core %d  uncore %d \n", (*newCoreFreq), (*newUncoreFreq));
		printf("memory bound new Freq core %d uncore %d \n\n", (*newCoreFreq), (*newUncoreFreq));
#endif
		return boundness;
#ifdef ROOFLINE
        } else if ( realIntensity > roofIntensity * ( 1 + ROOF_THREASHOLD ) ) {
#else
        } else if ( timeCoreBusy > timeMemBusy * (1+ROOF_THREASHOLD) ) {
#endif
		boundness = 1;
		(*newCoreFreq) = CORE_MAX_FREQ;
		//It may happen operation pattern changed from memory to compute  bound
		//or oversteering. 
		//if( i_coreFreq < CORE_MAX_FREQ ) {
		//	(*newCoreFreq) = CORE_MAX_FREQ;
		//	(*newUncoreFreq) = i_uncoreFreq + UNCORE_STEP_FREQ;
		//	if ((*newUncoreFreq) > UNCORE_MAX_FREQ) {
		//		(*newUncoreFreq) = UNCORE_MAX_FREQ;
		//	}
		//	printf("not right \n");
		//	//(*newUncoreFreq) = UNCORE_MAX_FREQ;
		//	return boundness;
		//}

		//calculate exeuction time if the core is at maxisum
		float timeAtMaxFreq = timeLatency * (float) i_uncoreFreq / (float) UNCORE_MAX_FREQ + timeCoreBusy;
		printf("compute bound time atMaxFreq %f latency %f coreBusy %f factor %f\n", timeAtMaxFreq, timeLatency, timeCoreBusy, (float) i_uncoreFreq / (float) UNCORE_MAX_FREQ  );

		if (timeAtMaxFreq * (1 + ROOF_THREASHOLD) < timeInterval ) {
#ifdef DEBUG
			printf("Oversterring, reset uncore by one\n");
#endif
			(*newUncoreFreq) = i_uncoreFreq + UNCORE_STEP_FREQ;
			return boundness;
		}

		(*newUncoreFreq) = i_uncoreFreq - UNCORE_STEP_FREQ; 
		while (1) {
			if ( (*newUncoreFreq) < UNCORE_MIN_FREQ ) {
				(*newUncoreFreq) = UNCORE_MIN_FREQ;
				break;
			}

			float factor =  (float) i_uncoreFreq / (float) (*newUncoreFreq);
			float newTimeLatency = timeLatency * factor ; 
#ifdef DEBUG
                        printf("timeInterval %f  timeCoreBusy %f timeMemBusy %f 1.05*timeAtMaxFreq %f new timeInterval %f noBottlenecktime %f\n", timeInterval, timeCoreBusy, timeMemBusy, 1.05*timeAtMaxFreq, newTimeLatency+timeCoreBusy, timeMemBusy * factor );
#endif
			if ( timeMemBusy * factor > timeCoreBusy || newTimeLatency + timeCoreBusy > (1+ROOF_THREASHOLD) * timeAtMaxFreq ) {
				(*newUncoreFreq) += UNCORE_STEP_FREQ;
				break;
			}
			(*newUncoreFreq) -= CORE_STEP_FREQ;
		}
#ifdef DEBUG
		printf("core bound new Freq core %d uncore %d \n\n", (*newCoreFreq), (*newUncoreFreq));
#endif
		return boundness;
        } else {
		(*newCoreFreq) = i_coreFreq;
		(*newUncoreFreq) = i_uncoreFreq;
		return boundness;
#ifdef DEBUG
		printf("balanced new Freq core %d uncore %d \n\n", (*newCoreFreq), (*newUncoreFreq));
#endif
        }
}


void NODE::finalize_perf_collection() {
	int i, j;
	for(i=0; i<NUM_CORES; i++) {
		for(j=0; j<NUM_CORE_EVENTS; j++) {
			ioctl( core_fds[i][j], PERF_EVENT_IOC_DISABLE, 0);
		}
	}
	for(i=0; i<NUM_SOCKETS; i++) {
		for(j=0; j<NUM_UNCORE_EVENTS; j++) {
			ioctl( uncore_fds[i][j], PERF_EVENT_IOC_DISABLE, 0);
		}
	}

	for(i=0; i<NUM_CORES; i++) {
		for(j=0; j<NUM_CORE_EVENTS; j++) {
        		close(core_fds[i][j]);
		}
	}
	for(i=0; i<NUM_SOCKETS; i++) {
		for(j=0; j<NUM_UNCORE_EVENTS; j++) {
        		close(uncore_fds[i][j]);
		}
	}
}

void NODE::set_core_freq(int coreID, int freq) {
	int socketID = coreID / NUM_CORES_PER_SOCKET; 
	int cid = coreID % NUM_CORES_PER_SOCKET; 

	write_msr_by_coord(socketID, cid, 0, MSR_CORE_FREQ, freq);
}

void NODE::set_socket_core_freq(int socketID, int freq) {
	int i; 

	for(i=0; i<NUM_CORES_PER_SOCKET; i++) {
		//printf("set core socket %d core %d : freq %d \n", socketID, i, freq);
		write_msr_by_coord(socketID, i, 0, MSR_CORE_FREQ, freq<<8);
	}
}

void NODE::set_node_core_freq(int freq) {
	int i; 
	for(i=0; i<NUM_CORES; i++) {
		set_core_freq(i, freq<<8);
	}
}

void NODE::set_socket_uncore_freq(int socketID, int freq) {
	//printf("set uncore socket %d freq %d \n", socketID, freq);
	write_msr_by_coord(socketID, 0, 0, MSR_UNCORE_FREQ, freq);
}

void NODE::set_node_uncore_freq(int freq) {
	int i; 
	for(i=0; i<NUM_SOCKETS; i++) {
		set_socket_uncore_freq(i, freq);
	}
}

void NODE::reset_node() {
	set_node_core_freq( CORE_MAX_FREQ ); 
	set_node_uncore_freq( UNCORE_MAX_FREQ );
}

NODE::NODE() {
	//numMetrics = NUM_CORE_METRICS + NUM_UNCORE_METRICS;
	init_collection();
	stop_uncore_collection();
	int i; 
	for(i=START_SOCKET; i<END_SOCKET; i++) {
		PKGEnergy[i] =  (float)uncorePerfValuesStop[i][8]; 
		DRAMEnergy[i] = (float)uncorePerfValuesStop[i][9]; 
	}
}

void NODE::finalize_collection() {
	int i; 
	stop_uncore_collection();
	for(i=START_SOCKET; i<END_SOCKET; i++) {
		PKGEnergy[i] =  uncorePerfValuesStop[i][8] - PKGEnergy[i];
		DRAMEnergy[i] =  uncorePerfValuesStop[i][9] - DRAMEnergy[i];

		PKGEnergy[i] = PKGEnergy[i] * 0.00000000023283064365386962890625;
		DRAMEnergy[i] = DRAMEnergy[i] * 0.00000000023283064365386962890625;
		printf("Energy of socket %d : PKG: %f DRAM %f \n", i, PKGEnergy[i], DRAMEnergy[i]);
	}
	finalize_libmsr_collection(); 
	finalize_perf_collection();
}

NODE::~NODE() {
	finalize_collection();
}

/*
int main() {
	NODE n; 
	int i,j; 
	for(j=0; j<10000; j++) {
		for(i=START_CORE; i<END_CORE; i++) {
			n.start_collection(i);
		}
		//0.01
		usleep(10000);
		//0.002
		//usleep(2000);
		//0.001
		//usleep(1000);
		//sleep(1);
		for(i=START_CORE; i<END_CORE; i++) {
			n.stop_core_collection(i);
		}

		n.calc_roof(0.01);
		//n.calc_roof(1);
		//n.calc_roof(0.002);
		//n.calc_roof(0.001);
		n.calc_roof(0.01);
	}
}
*/
