/*
 * BIN/EVO Project 2017 - Celular automata art
 * Author: Filip Gulan (xgulan00)
 * Mail: xgulan00@stud.fit.vutbr.cz
 * Date: 7.5.2017
 */

#ifndef BIN_EVO_ECA_H
#define BIN_EVO_ECA_H

#include <cmath>
#include <string.h>
#include <vector>
#include <iostream>
#include <bitset>
#include <algorithm>
#include <fstream>
#include <sstream>

#define ARG_ERR_MSG "Invalid arguments, see -h for help!"
#define SUCCESS 0
#define FAILURE 1
#define EMPTY -1
#define BEST 0

using namespace std;

typedef struct{
    int sum;
    int current;
    int next;
} rule;

typedef struct{
    int current;
    int next;
    int left;
    int right;
    int top;
    int bottom;
} finalRule;

typedef struct{
    vector<rule> rules;
    long int fitness;
} chromozome;

#endif //BIN_EVO_ECA_H
