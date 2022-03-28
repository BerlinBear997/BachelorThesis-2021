#!/usr/local_rwth/bin/zsh
for i in {0..23}
do
#	chmod o+rw /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_max_freq
done

echo 0 >/proc/sys/kernel/perf_event_paranoid

cd /work/bw579735/2019-ParCo/power_shifting/msr-safe
insmod msr-safe.ko
wl_file=$(printf 'wl_%.2x%x\n' $(lscpu | grep "CPU family:" | awk -F: '{print $2}') $(lscpu | grep "Model:" | awk -F: '{print $2}'))
echo $wl_file
cat whitelists/$wl_file > /dev/cpu/msr_whitelist
chmod 777 /dev/cpu/*/msr_safe
chmod 777 /dev/cpu/msr_batch
cd -

touch /w0/.likwid.no

cp /work/bw579735/2018-IPDPS/power_gov/power_gov /w0/tmp/
chmod 777 /w0/tmp/power_gov
setcap cap_sys_rawio+ep /w0/tmp/power_gov
