/*
 * Copyright (c) 2013-2017, Lawrence Livermore National Security, LLC.
 *
 * Produced at the Lawrence Livermore National Laboratory. Written by:
 *     Barry Rountree <rountree@llnl.gov>,
 *     Scott Walker <walker91@llnl.gov>, and
 *     Kathleen Shoga <shoga1@llnl.gov>.
 *
 * LLNL-CODE-645430
 *
 * All rights reserved.
 *
 * This file is part of libmsr. For details, see https://github.com/LLNL/libmsr.git.
 *
 * Please also read libmsr/LICENSE for our notice and the LGPL.
 *
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU Lesser
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cpuid.h"
#include "msr_core.h"
#include "msr_rapl.h"
#include "libmsr_error.h"


int main(int argc, char *argv)
{
    struct rapl_data *rd = NULL;
    uint64_t *rapl_flags = NULL;
    struct rapl_power_info info;
    uint64_t cores = 0;
    uint64_t threads = 0;
    uint64_t sockets = 0;
    int ri_stat = 0;
    unsigned i;
    float therm_power; 
    float power_cap;
    struct rapl_limit l1, l2; 
    uint64_t lock = 0x80000000;

    if (!sockets)
    {
        core_config(&cores, &threads, &sockets, NULL);
    }

    if (init_msr())
    {
        libmsr_error_handler("Unable to initialize libmsr", LIBMSR_ERROR_MSR_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

    ri_stat = rapl_init(&rd, &rapl_flags);
    if (ri_stat < 0)
    {
        libmsr_error_handler("Unable to initialize rapl", LIBMSR_ERROR_RAPL_INIT, getenv("HOSTNAME"), __FILE__, __LINE__);
        return -1;
    }

    get_rapl_power_info(0, &info);
    if (*rapl_flags & PKG_POWER_INFO){
    	printf("pkg_max_power(W) = %8.4lf pkg_thermal_power(W) = %8.4lf pkg_min_power(W) = %8.4lf\n", info.pkg_max_power, info.pkg_therm_power, info.pkg_min_power);
    }
    
    for(i=0; i<sockets; i++) {
    	uint64_t tmp; 
	printf("%x \n", MSR_PKG_POWER_LIMIT);
    	read_msr_by_coord(i, 0, 0, MSR_PKG_POWER_LIMIT, &tmp);
	printf("%llx \n", tmp);

	//tmp=0x0000000000000000; 
	tmp=0x00385a0001484b0;
	printf("%llx \n", tmp);

    	write_msr_by_coord(i, 0, 0, MSR_PKG_POWER_LIMIT, tmp);
    	read_msr_by_coord(i, 0, 0, MSR_PKG_POWER_LIMIT, &tmp);
	printf("%llx \n\n", tmp);

	if (get_pkg_rapl_limit(i, &l1, &l2) == 0){
    		if (l1.bits & 0x8000000000000000){
			fprintf(stderr, "Warning: <libmsr> MSR register locked on this architecture: check_for_locks(): Power limit 1 [0:23] of MSR_PKG_POWER_LIMIT (0x610) is locked, writes will be ignored: %s:%s::%d\n", getenv("HOSTNAME"), __FILE__, __LINE__);
		}
    		printf("socket %d current lower power limit  bits %lx seconds %f watts %f \n", i, l1.bits, l1.seconds, l1.watts);
    		//printf("socket %d current upper power limit  bits %x seconds %f watts %f \n", i, l2.bits, l2.seconds, l2.watts);
    	}
    }

    finalize_msr();
    return 0;
}
