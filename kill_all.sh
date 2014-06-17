#!/bin/sh

for i in feature_extract_cpp imitator elam3 'python supervisor.py'
do
  echo ${i}
  echo `ps -e | grep "${i}" | awk '{print $1}'`
  kill `ps -e | grep "${i}" | awk '{print $1}'`
done

