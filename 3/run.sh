#!/bin/bash

cpp_files="main.cpp"
for file in $(find src -name "*.cpp" -type f | grep -v "_test.cpp" | grep -v "test_"); do
    cpp_files="$cpp_files $file"
done

include_flags=""
for dir in $(find src -type d); do
    include_flags="$include_flags -I$dir"
done


g++ $include_flags $cpp_files -o main -std=c++17
./main