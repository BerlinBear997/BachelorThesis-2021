//define the broadwel hardware
	#ifndef DEF_HW
#define DEF_HW


#ifdef HASWELL
#define POWER_CAP 53
#define UNCORE_MAX_FREQ 30
#define UNCORE_MIN_FREQ 12
#define UNCORE_STEP_FREQ 1
#define CORE_MAX_FREQ 28
#define CORE_MAX_FREQ_NO_TURBO 23
#define CORE_MIN_FREQ 12
#define CORE_STEP_FREQ  1
#define MSR_CORE_FREQ 0x199

//a node with two sockets
#define NUM_CORES 36
#define NUM_SOCKETS 2
#define NUM_CORES_PER_SOCKET 18

//the start socket being managed
#define START_SOCKET 1
//the end socket is excluded
#define END_SOCKET 2
//cores to be measured
#define START_CORE START_SOCKET*NUM_CORES_PER_SOCKET
//the end core is excluded
#define END_CORE START_CORE+NUM_CORES_PER_SOCKET

#define NUM_CORE_EVENTS 6
#define NUM_CORE_METRICS NUM_CORE_EVENTS

#define NUM_UNCORE_EVENTS 10

#define NUM_STABLE_ITERATIONS 3
#define NUM_METRICS 5

#define NUM_LIBMSR_CTL_EVENTS 37
#define NUM_LIBMSR_EVENTS 19

#define EVENT_LABELS "cyc,ref_cyc,ldm-pend,stall-ldm-pend,pkg-energy"



#endif


#ifdef BROADWELL
#define POWER_CAP 53
#define UNCORE_MAX_FREQ 27
#define UNCORE_MIN_FREQ 12
#define UNCORE_STEP_FREQ 1
#define CORE_MAX_FREQ 25
#define CORE_MAX_FREQ_NO_TURBO 22
#define CORE_MIN_FREQ 12
#define CORE_STEP_FREQ  1
#define MSR_CORE_FREQ 0x199

//a node with two sockets
#define NUM_CORES 24
#define NUM_SOCKETS 2
#define NUM_CORES_PER_SOCKET 12

//the start socket being managed
#define START_SOCKET 1
//the end socket is excluded
#define END_SOCKET 2
//cores to be measured
#define START_CORE START_SOCKET*NUM_CORES_PER_SOCKET
//the end core is excluded
#define END_CORE START_CORE+NUM_CORES_PER_SOCKET

#define NUM_CORE_EVENTS 6
#define NUM_CORE_METRICS NUM_CORE_EVENTS

#define NUM_UNCORE_EVENTS 10

#define NUM_STABLE_ITERATIONS 3
#define NUM_METRICS 5

#define NUM_LIBMSR_CTL_EVENTS 25
#define NUM_LIBMSR_EVENTS 13

#define EVENT_LABELS "cyc,ref_cyc,ldm-pend,stall-ldm-pend,pkg-energy"

#endif

#ifdef SKYLAKE
#define UNCORE_MAX_FREQ 24
#define UNCORE_MIN_FREQ 12
#define CORE_MAX_FREQ 3700000
#define CORE_MIN_FREQ 1000000

#define NUM_CORES 48
#define NUM_SOCKETS 2 
#define NUM_CORES_PER_SOCKET 24

#define NUM_CORE_EVENTS 8
//#define NUM_CORE_EVENTS 8 // SIMD does not work on SKYLAKE
#define NUM_CORE_METRICS NUM_CORE_EVENTS

#define NUM_UNCORE_EVENTS 14
#define NUM_UNCORE_METRICS 3

#define NUM_STABLE_ITERATIONS 1
#define NUM_METRICS 5

#define EVENT_LABELS "ins,cyc,cache-ref,cache-miss,branch-miss,stall-l2-pend,stall-ldm-pend,l2_miss,pkg-energy,dram-energy,dram-vol"
#endif

#endif
