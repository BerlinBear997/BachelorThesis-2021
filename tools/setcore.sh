#!/bin/bash


max_core="2800000"
min_core="1200000"
core_freq=$1	
for i in {0..71}
do
	echo "set core freq" $i,$core_freq 
	echo $core_freq > /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_max_freq
done

