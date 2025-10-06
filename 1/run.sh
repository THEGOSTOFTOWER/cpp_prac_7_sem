#! /bin/bash
rm -rf logs
g++ main.cpp -fcoroutines --std=c++23 -o main.o
./main.o