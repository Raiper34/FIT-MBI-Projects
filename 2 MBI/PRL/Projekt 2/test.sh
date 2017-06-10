#!/bin/bash
# PRL Project2 2017 - Mesh Multiplication
# Author: Filip Gulan (xgulan00)
# Mail: xgulan00@stud.fit.vutbr.cz
# Date: 27.4.2017

mat1=$(head -n1 mat1)
mat2=$(head -n1 mat2)

cpus=$((mat1*mat2))

mpic++ --prefix /usr/local/share/OpenMPI -o mm mm.cpp -std=c++0x
mpirun --prefix /usr/local/share/OpenMPI -np $cpus mm
rm -f mm
