#!/bin/bash
#rm ./description.txt
TIMESTAMP="data_"$(date +%m.%d.%Y_%H.%M.%S)
#echo "Date: "$(date +%m/%d/%Y) >> description.txt
#echo "Time: "$(date +%H:%M:%S) >> description.txt
mkdir $TIMESTAMP
cp ./e? ./$TIMESTAMP
cp ./description.txt ./$TIMESTAMP
