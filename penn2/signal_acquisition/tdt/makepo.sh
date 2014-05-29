g++ ./main.cpp  ../../libs/inih/cpp/INIReader.cpp ../../libs/inih/ini.c -O3 -std=c++11 -o PO8eBroadcast -l PO8eStreaming -lzmq -lrt -lpthread
#sudo chmod 4777 ./PO8eBroadcast
