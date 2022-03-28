#define _GNU_SOURCE 1

#include <asm/unistd.h>
#include <fcntl.h>
#include <linux/perf_event.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

#include <chrono>


#include <cstdint>
#include "node.hpp"
using namespace std;
using namespace std::chrono;
NODE *p;
int fd;
high_resolution_clock::time_point start;

long perf_event_open(struct perf_event_attr* event_attr, pid_t pid, int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, event_attr, pid, cpu, group_fd, flags);
}
/*
void perf_event_handler(int signum, siginfo_t* info, void* ucontext) {
    static int count = 0;
    high_resolution_clock::time_point stop = high_resolution_clock::now();
    double d =  duration_cast<duration<double>>(stop - start).count();
    duration_cast<duration<double>>(high_resolution_clock::now() - start).count();
    printf("overflow %f\n", d);

//    p->calc_roof( d );

    start = high_resolution_clock::now();
    ioctl(info->si_fd, PERF_EVENT_IOC_REFRESH, 1);
    printf("--------------%d\n", count++);


}
*/

int main(int argc, char** argv){
	NODE node; 
	p = &node; 
	int i,j; 
	int coreFreq;
	int uncoreFreq;
	float timeInterval;
	for(i=START_SOCKET;i<END_SOCKET;i++){
		node.set_socket_core_freq(i,CORE_MAX_FREQ);
		node.set_socket_uncore_freq(i, UNCORE_MAX_FREQ);
	}

        while(1){
   
	
		for(i=START_CORE; i<END_CORE; i++) {

			p->start_collection(i);
		}
	
		start = high_resolution_clock::now();

	
		usleep(300000);
//	 p->calc_roof(timeInterval,&coreFreq, &uncoreFreq);
		for(i=START_CORE; i<END_CORE; i++){
			p->stop_core_collection(i);
		}
		p->stop_uncore_collection();
		high_resolution_clock::time_point tmp = high_resolution_clock::now();
		timeInterval = duration_cast<duration<double>>(tmp-start).count();
	
		
		p->calc_roof(timeInterval,&coreFreq, &uncoreFreq);
    	}	
   
}

