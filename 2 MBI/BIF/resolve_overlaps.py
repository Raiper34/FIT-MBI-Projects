#!/usr/bin/env python3.6
# -*- coding: utf-8 -*-
# BIF Project 2017
# Author: Filip Gulan (xgulan00)
# Mail: xgulan00@stud.fit.vutbr.cz
# Date: 5.5.2017
# Name: resolve_overlaps.py

import sys
import datetime
from operator import itemgetter

"""
Implementation of Weigh interval scheduling algorithm
First compute J function values, then compute optimal value and then use backtracking to find solution
Input: data to find solution
Output: array of indexes that are solution
"""
def weightIntervalScheduling(data):
    #Scores or weights V
    dataV = []
    dataV.append(0)
    for item in data:
        dataV.append(item['score'])

    #J function values
    dataJ = []
    dataJ.append(0)
    for i in range(len(data)): #calculate J values for all datas
        j = i - 1
        toAppend = 0 #if we do not find, we append only 0
        while j >= 0: #we search for first item that has lower end than actual item start and lower index
            if data[j]['end'] < data[i]['start']:
                toAppend = j + 1
                break
            j -= 1
        dataJ.append(toAppend)

    # Calculation optimal value
    dataM = []
    dataM.append(0)
    for i in range(len(dataV)):
        if i == 0:
            continue
        if dataV[i] + dataM[dataJ[i]] > dataM[i - 1]:
            dataM.append(dataV[i] + dataM[dataJ[i]])
        else:
            dataM.append(dataM[i - 1])

    #print(dataM[len(dataM) - 1])

    #Solution
    dataS = []
    j = len(dataM) - 1
    while j != 0: #original backtracking was made by resursion, but this is better for me
        if dataV[j] + dataM[dataJ[j]] > dataM[j - 1]:
            dataS.append(j - 1) # j - 1 because original data array do not have 0 at the start
            j = dataJ[j]
        else:
            j -= 1

    return dataS

"""
Optimize gff3
Read file by line, find best nonoverlapping solution and print it to stdout
Input: file to read
"""
def gff3Optimizer(input):
    data = {}
    for line in input: #itarete trought file lines
        if line[0:2] == "##": #it is comment
            pass
        else: #it is data line
            values = [value for value in line.split('\t')] #split datas separated by tabs into array
            index = values[0] + values[2] + values[6] #unique index for array of datas that can be overlapped
            if data.get(index) is None: #if index does not work, we create one
                data[index] = {}
                data[index]['data'] = []
                data[index]['seqId'] = values[0]
                data[index]['type'] = values[2]
                data[index]['strand'] = values[6]
            data[index]['data'].append({'start': int(values[3]), 'end': int(values[4]), 'score': int(values[5]), 'source': values[1], 'phase': values[7].replace("\n", "")})  # start, end, score

    print("##gff-version 3\n##date " + datetime.datetime.now().strftime("%Y-%m-%d")) #print gff3 file header
    for key, item in data.items():
        data[key]['data'] = sorted(data[key]['data'], key=itemgetter('end')) #sort by ascending finish time
        overalScore = 0 #debug purpose only
        solution = weightIntervalScheduling(data[key]['data'])
        for index in solution: #we iterate trought array of indexes, that are solutions
            overalScore += data[key]['data'][index]['score'] #debug purpose only
            item = data[key]['data'][index]
            print(data[key]['seqId'] + '\t' + item['source'] + '\t' + data[key]['type'] + '\t' + str(item['start']) + '\t' + str(item['end']) + '\t' + str(item['score']) + '\t' + data[key]['strand'] + '\t' + item['phase'])
        #print(overalScore) #only for debug purpose

#----------- Main ------------#

#Arguments
if len(sys.argv) != 2: #there is no one required argument
    print("Exactly one argument is required!", file=sys.stderr)
    exit(1)

inputFileName = sys.argv[1]

#Open input file
try:
    with open(inputFileName, 'r') as inputFile:
        gff3Optimizer(inputFile)
except (OSError, IOError, FileNotFoundError): #file can not be opened
    print("Can not open file!", file=sys.stderr)
    sys.exit(1)
