#!/bin/bash

if test -d build-test;
then 
	rm -rf build-test
fi

mkdir build-test
cd build-test

cmake -DCOMPILE_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make 
