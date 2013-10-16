cpufreq-set -r -g performance
insmod ./Plx9054.ko
./mkDevs
chmod 777 /dev/plx
chmod 777 /dev/plx/*

