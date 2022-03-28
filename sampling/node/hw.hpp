//define the broadwel hardware
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h> 
#include <stdint.h>

#ifndef HW_HPP
#define HW_HPP
typedef struct {
	int type;
	int config;
} PERF_EVENT;

typedef struct {
	uint64_t idx; 
	uint64_t enable; 
	uint64_t disable;
} LIBMSR_CONTROL_EVENT;

typedef struct {
	//select register
	uint64_t s_idx; 
	uint64_t config;
	//value register
	uint64_t v_idx; 
	uint64_t value;
} LIBMSR_EVENT;

typedef struct{
	uint64_t idex; 
	uint64_t config;
} LIBMSR_CLEAR_EVENT;
#ifdef HASWELL

float inverseclock=0;

#define MSR_UNCORE_FREQ 0x620
#define MSR_UNCORE_FREQ_READ 0x620
//#define INVERSE_CLOCK 4.545444e-10
#define INVERSE_CLOCK 4.3668122e-10
#define CLOCK 2.29E9

//float FULL_L3_BW[] =  {2.00E+11,2.15E+11,2.31E+11,2.45E+11,2.57E+11,2.72E+11,2.90E+11,3.05E+11,
//3.17E+11,3.26E+11,3.37E+11,3.48E+11,3.62E+11,3.70E+11,3.79E+11,3.87E+11};
//float FULL_L3_BW[]={ 1.24E+11, 1.29E+11, 1.33E+11, 1.35E+11,1.37E+11, 1.39E+11,1.41E+11, 
//1.42E+11, 1.45E+11, 1.45E+11,1.46E+11, 1.47E+11,1.48E+11,1.49E+11,1.50E+11,1.51E+11,1.52E+11,1.52E+11,1.52E+11};  
float FULL_L3_BW[]={ 1.81E+11, 1.88E+11, 1.93E+11, 1.98E+11,2.01E+11, 2.08E+11,2.10E+11, 2.12E+11,
 2.13E+11, 2.14E+11,2.18E+11, 2.20E+11,2.21E+11,2.22E+11,2.23E+11,2.24E+11,2.24E+11,2.24E+11,2.24E+11};  

//to be modified
//float FULL_RAM_BW[] = {5.0082e+10,4.64E+10,4.64E+10,4.64E+10,4.64E+10,4.64E+10,4.64E+10,4.64E+10,4.64>
//4.64E+10,4.64E+10,4.64E+10,4.64E+10,4.64E+1
//float FULL_RAM_BW[] = {5.00E+10,5.00E+10,5.00E+10,5.00E+10,5.00E+10,5.00E+10,5.00E+10,5.00E+10,
//5.00E+10,5.00E10,5.00E10,5.00E+10,5.00E+10,5.00E+10,5.00E+10,5.00E+10,5.00E+10,5.00E+10};
float FULL_RAM_BW[]={3.56E+10,  3.78E+10, 4.03E+10, 4.20E+10,  4.34E+10, 4.43E+10, 4.57E+10,
 4.67E+10, 4.78E+10, 4.90E+10, 4.92E+10, 4.98E+10,5.02E+10, 5.04E+10, 5.05E+10, 5.06E+10, 5.07E+10, 5.08E+10, 5.10E+10};

float L3_LATENCY_AT_CORE_SCALING[] = {11.002, 10.18, 9.365, 8.848, 8.194, 7.743, 7.275, 6.976, 6.595, 6.219, 5.983, 5.275, 5.175, 5.117, 5.117, 5.117, 5.117};

float DRAM_LATENCY_AT_CORE_SCALING[] = {14.907, 14.378, 13.921, 13.634, 13.286, 13.033, 12.823, 12.725, 12.604, 12.546, 12.436, 12.238, 12.237, 12.235, 12.23, 12.23, 12.23};

float L3_LATENCY_AT_UNCORE_SCALING[] = {5.93, 5.623, 5.584, 5.512, 5.379, 5.346, 5.35, 5.395, 5.423, 5.334, 5.418, 5.317, 5.303, 5.264, 5.245, 5.253, 5.253, 5.253, 5.253}; 

float DRAM_LATENCY_AT_UNCORE_SCALING[] = {25.377, 23.453, 21.737, 20.429, 19.226, 17.988, 17.141, 16.289, 15.526, 14.851, 14.319, 13.766, 13.219, 12.838, 12.517,	12.241, 12.241, 12.241, 12.241};


PERF_EVENT core_events[NUM_CORE_EVENTS] = {
        //instructions
        {PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS}, \
        //cycles
        {PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES}, \
        //ref cycles
        {PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES}, \
        //stall ld2 pending: cycle_activity.stall_l2m_pending
        {PERF_TYPE_RAW, 0x54105A3}, \
        //stall ldm pending: cycle_activity.stalls_ldm_pending
        {PERF_TYPE_RAW, 0x64106A3}, \
        //stall cycle_activity.stalls_all
        {PERF_TYPE_RAW, 0x44104A3}
        //cycle ld2 pending: cycle_activity_cycle_l2m_pending
        //{PERF_TYPE_RAW, 0x14101A3}, \
        //cycle ldm pending: cycle_activity_cycle_ldm_pending
        //{PERF_TYPE_RAW, 0x24102A3}, \

        //L3 all
        //{PERF_TYPE_RAW, 0x4124D1}, \
        //L3 misses
        //{PERF_TYPE_RAW, 0x4120D1}, \
        //l2 line in
        //r4140F0
        //l2 victom
        //r4107F1
};

PERF_EVENT uncore_events[NUM_UNCORE_EVENTS] = {
        {16, 0x500304},
        {16, 0x500C04},
        {17, 0x500304},
        {17, 0x500C04},
        {18, 0x500304},
        {18, 0x500C04},
        {19, 0x500304},
        {19, 0x500C04},
        //pkg energy
        {46, 0x2},
        //dram energy
        {46, 0x3}
};

LIBMSR_CONTROL_EVENT libmsr_control_events[NUM_LIBMSR_CTL_EVENTS] = {
        //CBOX
        {0xE05, 0x3E0000, 0x0},
        {0xE15, 0x3E0000, 0x0},
        {0xE25, 0x3E0000, 0x0},
        {0xE35, 0x3E0000, 0x0},
        {0xE45, 0x3E0000, 0x0},
        {0xE55, 0x3E0000, 0x0},
        {0xE65, 0x3E0000, 0x0},
        {0xE75, 0x3E0000, 0x0},
        {0xE85, 0x3E0000, 0x0},
        {0xE95, 0x3E0000, 0x0},
        {0xEA5, 0x3E0000, 0x0},
     
	{0xEB5, 0x3E0000, 0x0},
	{0xEC5, 0x3E0000, 0x0},
        {0xED5, 0x3E0000, 0x0},
        {0xEE5, 0x3E0000, 0x0},
        {0xEF5, 0x3E0000, 0x0},
        {0xF05, 0x3E0000, 0x0},
        {0xF15, 0x3E0000, 0x0},
	

        {0xE00, 0x0, 0x0},
        {0xE10, 0x0, 0x0},
        {0xE20, 0x0, 0x0},
        {0xE30, 0x0, 0x0},
        {0xE40, 0x0, 0x0},
        {0xE50, 0x0, 0x0},
        {0xE60, 0x0, 0x0},
        {0xE70, 0x0, 0x0},
        {0xE80, 0x0, 0x0},
        {0xE90, 0x0, 0x0},
        {0xEA0, 0x0, 0x0},
       
	
	{0xEB0, 0x0, 0x0},
	{0xEC0, 0x0, 0x0},
        {0xED0, 0x0, 0x0},
        {0xEE0, 0x0, 0x0},
        {0xEF0, 0x0, 0x0},
        {0xF00, 0x0, 0x0},
        {0xF10, 0x0, 0x0},
      
  //uncore overflow
        {0x701, 0x0, 0x0},
        //uncore control
        //{0x700, 0x0, 0x0}
};



LIBMSR_EVENT libmsr_events[NUM_LIBMSR_EVENTS]{
        //CBOX
        {0xE01, 0x401134, 0xE08},
        {0xE11, 0x401134, 0xE18},
        {0xE21, 0x401134, 0xE28},
        {0xE31, 0x401134, 0xE38},
        {0xE41, 0x401134, 0xE48},
        {0xE51, 0x401134, 0xE58},
        {0xE61, 0x401134, 0xE68},
        {0xE71, 0x401134, 0xE78},
        {0xE81, 0x401134, 0xE88},
        {0xE91, 0x401134, 0xE98},
        {0xEA1, 0x401134, 0xEA8},
        {0xEB1, 0x401134, 0xEB8},
	{0xEC1, 0x401134, 0xEC8},
        {0xED1, 0x401134, 0xED8},
        {0xEE1, 0x401134, 0xEE8},
        {0xEF1, 0x401134, 0xEF8},
        {0xF01, 0x401134, 0xF08},
        {0xF11, 0x401134, 0xF18},

        //UBOX
        {0x703, 0x500000, 0x704}
};

#endif




#ifdef BROADWELL
float inveseclock=0;

#define MSR_UNCORE_FREQ 0x620
#define MSR_UNCORE_FREQ_READ 0x620
#define INVERSE_CLOCK 4.545444e-10
#define CLOCK 2.2E9


float FULL_L3_BW[] =  {2.00E+11,2.15E+11,2.31E+11,2.45E+11,2.57E+11,2.72E+11,2.90E+11,3.05E+11,
3.17E+11,3.26E+11,3.37E+11,3.48E+11,3.62E+11,3.70E+11,3.79E+11,3.87E+11}; 
//to be modified
float FULL_RAM_BW[] = {4.17E+10,4.43E+10,4.65E+10,4.87E+10,5.03E+10,5.19E+10,5.29E+10,5.48E+10,
5.51E+10,5.62E+10,5.67E+10,5.76E+10,5.75E+10,5.87E+10,5.88E+10,5.95E+10};

float L3_LATENCY_AT_CORE_SCALING = {11.002, 10.18, 9.365, 8.848, 8.194, 7.743, 7.275, 6.976, 6.595, 6.219, 5.983, 5.275, 5.175, 5.117};

float DRAM_LATENCY_AT_CORE_SCALING = {14.907, 14.378, 13.921, 13.634, 13.286, 13.033, 12.823, 12.725, 12.604, 12.546, 12.436, 12.238, 12.237, 12.235};

float L3_LATENCY_AT_UNCORE_SCALING = {5.93, 5.623, 5.584, 5.512, 5.379, 5.346, 5.35, 5.395, 5.423, 5.334, 5.418, 5.317, 5.303, 5.264, 5.245, 5.253}; 

float DRAM_LATENCY_AT_UNCORE_SCALING = {25.377, 23.453, 21.737, 20.429, 19.226, 17.988, 17.141, 16.289, 15.526, 14.851, 14.319, 13.766, 13.219, 12.838, 12.517,	12.241};



//PERF_EVENT core_events[NUM_CORE_EVENTS] = {
PERF_EVENT core_events[NUM_CORE_EVENTS] = {
	//instructions
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS}, \
	//cycles
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES}, \
	//ref cycles
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES}, \
	//stall ld2 pending: cycle_activity.stall_l2m_pending
	{PERF_TYPE_RAW, 0x54105A3}, \
	//stall ldm pending: cycle_activity.stalls_ldm_pending
	{PERF_TYPE_RAW, 0x64106A3}, \
	//cycle ld2 pending: cycle_activity_cycle_l2m_pending
	//{PERF_TYPE_RAW, 0x14101A3}, \
	//cycle ldm pending: cycle_activity_cycle_ldm_pending
	//{PERF_TYPE_RAW, 0x24102A3}, \
	
	//L3 all
	//{PERF_TYPE_RAW, 0x4124D1}, \
	//L3 misses
	//{PERF_TYPE_RAW, 0x4120D1}, \
	//stall cycle_activity.stalls_all
	{PERF_TYPE_RAW, 0x44104A3} 
	//l2 line in
	//r4140F0
	//l2 victom
	//r4107F1
};

PERF_EVENT uncore_events[NUM_UNCORE_EVENTS] = {
	{18, 0x500304}, 
	{18, 0x500C04}, 
	{19, 0x500304}, 
	{19, 0x500C04}, 
	{20, 0x500304}, 
	{20, 0x500C04}, 
	{21, 0x500304}, 
	{21, 0x500C04}, 
	//pkg energy
	{10, 0x2}, 
	//dram energy 
	{10, 0x3}
};

LIBMSR_CONTROL_EVENT libmsr_control_events[NUM_LIBMSR_CTL_EVENTS] = {
	//CBOX
	{0xE05, 0x3E0000, 0x0},
	{0xE15, 0x3E0000, 0x0},
	{0xE25, 0x3E0000, 0x0},
	{0xE35, 0x3E0000, 0x0},
	{0xE45, 0x3E0000, 0x0},
	{0xE55, 0x3E0000, 0x0},
	{0xE65, 0x3E0000, 0x0},
	{0xE75, 0x3E0000, 0x0},
	{0xE85, 0x3E0000, 0x0},
	{0xE95, 0x3E0000, 0x0},
	{0xEA5, 0x3E0000, 0x0},
	{0xEB5, 0x3E0000, 0x0}, 
	{0xE00, 0x0, 0x0}, 
	{0xE10, 0x0, 0x0}, 
	{0xE20, 0x0, 0x0}, 
	{0xE30, 0x0, 0x0}, 
	{0xE40, 0x0, 0x0}, 
	{0xE50, 0x0, 0x0}, 
	{0xE60, 0x0, 0x0}, 
	{0xE70, 0x0, 0x0}, 
	{0xE80, 0x0, 0x0}, 
	{0xE90, 0x0, 0x0}, 
	{0xEA0, 0x0, 0x0}, 
	{0xEB0, 0x0, 0x0},
	//uncore overflow
	{0x701, 0x0, 0x0},
	//uncore control
	//{0x700, 0x0, 0x0}
};

LIBMSR_EVENT libmsr_events[NUM_LIBMSR_EVENTS]{
	//CBOX
	{0xE01, 0x401134, 0xE08},
	{0xE11, 0x401134, 0xE18},
	{0xE21, 0x401134, 0xE28},
	{0xE31, 0x401134, 0xE38},
	{0xE41, 0x401134, 0xE48},
	{0xE51, 0x401134, 0xE58},
	{0xE61, 0x401134, 0xE68},
	{0xE71, 0x401134, 0xE78},
	{0xE81, 0x401134, 0xE88},
	{0xE91, 0x401134, 0xE98},
	{0xEA1, 0x401134, 0xEA8},
	{0xEB1, 0x401134, 0xEB8},
	//UBOX
	{0x703, 0x500000, 0x704}
};

#endif

#ifdef SKYLAKE
float inveseclock=0;

#define MSR_UNCORE_FREQ 0x620
#define MSR_UNCORE_FREQ_READ 0x620

EVENT core_events[NUM_CORE_EVENTS] = {
	//instructions
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS}, \
	//cycles
	{PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES}, \
	//cache-refenrences: cache-references
	{PERF_TYPE_HARDWARE, 0x2}, \
	//cache-misses: cache-misses
	{PERF_TYPE_HARDWARE, 0x3}, \
	//branch misses: branch-misses
	{PERF_TYPE_HARDWARE, 0x5}, \
	//simd:fp_arith_inst_retired.packed
	//{PERF_TYPE_RAW, 0x40c7}, \
	//port 0: uops_dispatched_port.port_0
	//{PERF_TYPE_RAW, 0x1a1},\
	//offcore_response.pf_l2_rfo.any_response
	//{PERF_TYPE_RAW, 0x1b7},\
	//stall l2 pending: cycle_activity.stalls_l2_pending
	{PERF_TYPE_RAW, 0x50005a3}, \
	//stall ldm pending: cycle_activity.stalls_ldm_pending
	{PERF_TYPE_RAW, 0x144114A3}, \
	//l2 request: l2_trans.all_requests
	//{PERF_TYPE_RAW, 0x80f0}, \
	//l2 misses: mem_load_uops_retired.l2_miss
	{PERF_TYPE_RAW, 0x413F24}, \
};

EVENT uncore_events[NUM_UNCORE_EVENTS] = {
	//pkg energy
	{11, 0x2}, \
	//dram energy
	{11, 0x3}, \
	//imc
	{14, 0x304}, \
	{14, 0xc04}, \
	{15, 0x304}, \
	{15, 0xc04}, \
	{16, 0x304}, \
	{16, 0xc04}, \
	{17, 0x304}, \
	{17, 0xc04}, \
	{18, 0x304}, \
	{18, 0xc04}, \
	{19, 0x304}, \
	{19, 0xc04}, \
	//cbox clock tick, unc_c_clockticks
	//cbox read misses 
	//unc_p_clockticks PCU clock tick
};
#endif
#endif
