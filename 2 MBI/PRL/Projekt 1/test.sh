#!/bin/bash
# PRL Project1 2017 - Enumeration sort test script
# Author: Filip Gulan (xgulan00)
# Mail: xgulan00@stud.fit.vutbr.cz
# Date: 31.3.2017

if [ "$#" -gt 1 ]; then #there are more parameters than 1
    echo "Illegal number of parameters"
    exit 1
elif [ "$#" -eq 1 ]; then #there is one parameter
    numbers=$1;
else #there is no parameter
    numbers=5;
fi

#create random numbers file
dd if=/dev/random bs=1 count=$numbers of=numbers 2>/dev/null

#compile
mpic++ --prefix /usr/local/share/OpenMPI -o es es.cpp

#number of processes neeed to be equal to number of numbers + 1
numbers=$((numbers + 1))

#run
mpirun --prefix /usr/local/share/OpenMPI -np $numbers es

#remove
rm -f es numbers
