/*
 * KKO Project 2017 - GIF to BMP Converter
 * Author: Filip Gulan (xgulan00)
 * Mail: xgulan00@stud.fit.vutbr.cz
 * Date: 7.5.2017
 * Name: main.cpp
 * Description: Main application
 */

#include <iostream>
#include <string.h>
#include <fstream>
#include <bitset>
#include <tgmath.h>
#include <vector>
#include <iomanip>
#include "gif2bmp.h"

#define SUCCESS 0
#define FAILURE 1
#define HELP_SUCCESS -1
#define ARG_ERR_MSG "Invalid arguments!"
#define FILE_OPEN_ERR_MSG "Can not open file!"

using namespace std;


/**
 * Print help to stdin
 */
void printHelp()
{
    cout << "-------------------------------------------------------------------" << endl;
    cout << "KKO Project 2017 - GIF to BMP Converter" << endl;
    cout << "Author: Filip Gulan" << endl;
    cout << "E-mail: xgulan00@stud.fit.vutbr.cz" << endl;
    cout << "-------------------------------------------------------------------" << endl;
    cout << "Parameters:" << endl;
    cout << "   -i <ifile>: name of input file <ifile>. If parameter is missing, input is stdin." << endl;
    cout << "   -o <ofile>: name of output file <ofile>. If parameter is missing, output is stdout." << endl;
    cout << "   -l <logfile>: name of log file for output message <logfile>. If parameter is missing then output message is ignored." << endl;
    cout << "   -h: print this help (can not be combined with others parameters)" << endl;
    cout << "-------------------------------------------------------------------" << endl;
    return;
}

/**
 * Check if arguments are in correct format, if no return FAILURE, if yes return SUCCESS or HELP_SUCCESS
 * @param help
 * @param inputFile
 * @param outputFile
 * @param logFile
 * @param lastParam
 * @return
 */
int checkArguments(bool help, string inputFile, string outputFile, string logFile, string lastParam)
{
    if(help) //help argument set
    {
        if(inputFile.compare("") != 0 || outputFile.compare("") != 0 || logFile.compare("") != 0) //help can not be combined with others
        {
            cerr << ARG_ERR_MSG << endl;
            return FAILURE;
        }
        else //there is only help argument, we print help
        {
            printHelp();
            return HELP_SUCCESS;
        }
    }
    else if(lastParam.compare("") != 0) //there are - argument without second path argument
    {
        cerr << ARG_ERR_MSG << endl;
        return FAILURE;
    }
    return SUCCESS;
}

/**
 * Main function
 * @param argc number of arguments
 * @param argv arguments
 * @return 0 in case of success, 1 otherwise
 */
int main(int argc, char *argv[])
{

    //Parse comandline arguments
    string lastParam = "";
    string inputFile = "";
    string outputFile = "";
    string logFile = "";
    bool help = false;
    for(int i = 1; i < argc; i++)
    {
        if(lastParam.compare("") == 0) //we check por - parameters
        {
            if(strcmp(argv[i], "-i") == 0) //input file
            {
                lastParam = "-i";
            }
            else if(strcmp(argv[i], "-o") == 0) //output file
            {
                lastParam = "-o";
            }
            else if(strcmp(argv[i], "-l") == 0) //log file
            {
                lastParam = "-l";
            }
            else if(strcmp(argv[i], "-h") == 0) //help
            {
                //lastParam = "-h";
                help = true;
            }
            else //Unkown argument
            {
                cerr << ARG_ERR_MSG << endl;
                return FAILURE;
            }
        }
        else if(lastParam.compare("-i") == 0) //path of input file
        {
            inputFile = argv[i];
            lastParam = "";
        }
        else if(lastParam.compare("-o") == 0) //path of output file
        {
            outputFile = argv[i];
            lastParam = "";
        }
        else if(lastParam.compare("-l") == 0) //path of log file
        {
            logFile = argv[i];
            lastParam = "";
        }
    }
    int returnCode = checkArguments(help, inputFile, outputFile, logFile, lastParam);
    if(returnCode != SUCCESS)
    {
        if(returnCode == HELP_SUCCESS)
        {
            return SUCCESS;
        }
        return returnCode;
    }

    //Input
    ifstream inputFileStream;
    if(inputFile.compare("") != 0)
    {
        inputFileStream.open(inputFile.c_str(), ios::binary);
        if(inputFileStream.fail())
        {
            cerr << FILE_OPEN_ERR_MSG << endl;
            return FAILURE;
        }
    }

    //Output
    ofstream outputFileStream;
    if(outputFile.compare("") != 0)
    {
        outputFileStream.open(outputFile.c_str());
        if(outputFileStream.fail())
        {
            cerr << FILE_OPEN_ERR_MSG << endl;
            return FAILURE;
        }
    }

    //Execute library
    tGIF2BMP gifInfo;
    returnCode = gif2bmp(&gifInfo, &inputFileStream, &outputFileStream);
    if(returnCode != SUCCESS)
    {
        return returnCode;
    }

    //Log
    if(logFile.compare("") != 0) //we want logfile
    {
        ofstream logFileStream;
        logFileStream.open(logFile.c_str());
        if(logFileStream.fail())
        {
            cerr << FILE_OPEN_ERR_MSG << endl;
            return FAILURE;
        }
        logFileStream << "login = xgulan00" << endl;
        logFileStream << "uncodedSize = " + to_string(gifInfo.bmpSize) << endl;
        logFileStream << "codedSize = " + to_string(gifInfo.gifSize) << endl;

    }

    return returnCode;
}