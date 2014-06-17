#!/bin/bash -x

cd penn2/feature_extraction/feature_extract_cpp/build/
./feature_extract_cpp &
cd -

cd penn2/signal_acquisition/imitator/build/
./imitator &
cd -

cd penn2/supervisor/elam3/
python supervisor.py &
cd -

cd penn2/graphics/elam3/build/
./elam3 &
cd -

cd penn2/libs/zeromq-matlab/
make
cd -

