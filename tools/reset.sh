#!/usr/local_rwth/bin/zsh
#broadwell
power_cap=105
tdp_power=105
min_uncore=12
max_uncore=27
#max_uncore=27
min_core="1200000"
#max_core="2900000"
max_core="2500000"

#skylake 
#power_cap=150
#tdp_power=150
#min_uncore=12
#max_uncore=24
#min_core="1000000"
#max_core="3700000"


/w0/tmp/power_gov -r TIME_WINDOW -s 0.01 -d PKG
echo "set power cap " $power_cap
/w0/tmp/power_gov -r POWER_LIMIT -s $power_cap -d PKG  

for i in {0..23}
do
	echo "set core freq" $i, $max_core
	echo $max_core > /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_max_freq
done

echo "set uncore freq"  $max_uncore
/work/bw579735/2021-IPDPS_arapl/arapl/tools/set_uncore $max_uncore
