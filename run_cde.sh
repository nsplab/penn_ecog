#!/bin/sh -x

ROOT=`pwd`

#mkdir -p penn2/analysis/Mosalam/canal/build
#cd penn2/analysis/Mosalam/canal/build
#cmake ..
#cd -
#cde make -f penn2/analysis/Mosalam/canal/build/Makefile

#cde penn2/feature_extraction/feature_extract_cpp/build/feature_extract_cpp
#cde ./run_all.sh
cde ./run_supervisor.sh 1 1 1 &
sleep 1
./run_signal_acquisition.sh 1 2 &
sleep 1
./run_graphics.sh 1 &

sleep 5
./run_supervisor.sh 1 1 1 &
sleep 1
cde ./run_signal_acquisition.sh 1 2 &
sleep 1
./run_graphics.sh 1 &

sleep 5
./run_supervisor.sh 1 1 1 &
sleep 1
./run_signal_acquisition.sh 1 2 &
sleep 1
cde ./run_graphics.sh 1 &

#cde ./run_filter.sh 
sleep 5
cde ./compile_zmq.sh

mkdir -p cde-package/cde-root/home/bryanhe/penn_ecog/penn2/filter
cp -r penn2/filter/matlab/ cde-package/cde-root/home/bryanhe/penn_ecog/penn2/filter/matlab
cp -r penn2/config/filter.cfg cde-package/cde-root/home/bryanhe/penn_ecog/penn2/config/filter.cfg
cp -r `which mex` cde-package/cde-root/home/bryanhe/penn_ecog/penn2/libs/zeromq-matlab/

