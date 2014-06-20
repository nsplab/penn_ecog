cpufreq-set -r -g performance
insmod ../signal_acquisition/tdt/Plx9054.ko
../signal_acquisition/tdt/mkDevs
chmod 777 /dev/plx
chmod 777 /dev/plx/*

