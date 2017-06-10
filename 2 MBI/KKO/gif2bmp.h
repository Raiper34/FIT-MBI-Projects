/*
 * KKO Project 2017 - GIF to BMP Converter
 * Author: Filip Gulan (xgulan00)
 * Mail: xgulan00@stud.fit.vutbr.cz
 * Date: 7.5.2017
 * Name: gif2bmp.h
 * Description: Gif2Bmp library header file
 */

#ifndef KKO_GIF2BMP_H
#define KKO_GIF2BMP_H

#include <cstdint>
#include <cstdio>
#include <string.h>
#include <fstream>
#include <iostream>

using namespace std;

//Structures
typedef struct{
    int64_t bmpSize;
    int64_t long gifSize;
} tGIF2BMP;

//Functions
int gif2bmp(tGIF2BMP *gif2bmp, ifstream *inputFile, ofstream *outputFile);


#endif //KKO_GIF2BMP_H
