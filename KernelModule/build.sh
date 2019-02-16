#!/bin/bash
cd build
mv ../Kernel.c Kernel.c
printf "Make:"
make -j 8
mv Kernel.c ../Kernel.c
cd ..
