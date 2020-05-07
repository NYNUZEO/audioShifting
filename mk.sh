#!/bin/bash
rm ./run
g++ -std=c++11 -g -ggdb -o run sonicMain.cpp sonic.cpp  `pkg-config --cflags --libs libavutil libavformat libavcodec libavfilter libswscale libswresample`
