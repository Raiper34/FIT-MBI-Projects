/*
 * PRL Project1 2017 - Enumeration sort
 * Author: Filip Gulan (xgulan00)
 * Mail: xgulan00@stud.fit.vutbr.cz
 * Date: 31.3.2017
 */

#include <iostream>
#include <mpi.h>
#include <vector>
#include <string>
#include <fstream>
#include <time.h>

#define OK 0
#define FAILURE 1
#define TAGX 1
#define TAGY 2
#define TAGZ 3
#define TAGO 4
#define INITC 1
#define INIT -1
#define MSG_SIZE 1
#define MASTER 0
#define FIRST_SLAVE 1
#define MEASURE false

using namespace std;

/**
 * Measure time and print result to std out, it make diff of start time and end time
 * Source: http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
 * @param timeStart
 * @param timeEnd
 */
void measureTime(timespec timeStart, timespec timeEnd)
{
    timespec timeTemp;
    if ((timeEnd.tv_nsec - timeStart.tv_nsec) < 0)
    {
        timeTemp.tv_sec = timeEnd.tv_sec - timeStart.tv_sec - 1;
        timeTemp.tv_nsec = 1000000000 + timeEnd.tv_nsec - timeStart.tv_nsec;
    } else
    {
        timeTemp.tv_sec = timeEnd.tv_sec - timeStart.tv_sec;
        timeTemp.tv_nsec = timeEnd.tv_nsec - timeStart.tv_nsec;
    }
    cout<< timeTemp.tv_sec << ":" << timeTemp.tv_nsec << endl;
}

/**
 * Main function
 * @param argc number of arguments on command line
 * @param argv "array" of arguments
 * @return 0 if success, 1 otherwise
 */
int main(int argc, char *argv[])
{
    int processesCount;
    int numbersCount;
    int processId;
    int lastProcess;

    MPI_Init(&argc, &argv); //initialize MPI
    MPI_Comm_size(MPI_COMM_WORLD,&processesCount); //get number of processes
    MPI_Comm_rank(MPI_COMM_WORLD,&processId); //get actual process id
    MPI_Status status;
    numbersCount = processesCount - 1;
    lastProcess = processesCount - 1;

    if(processId == 0) //master process BUS, who control all other
    {
        //Read numbers from file
        string numbersFileName = "numbers"; //name of file with number to work with
        int number;
        int j = 0;
        vector<int> numbers;
        ifstream inputFileStream;
        inputFileStream.open(numbersFileName.c_str());
        //Read while there is any character
        while(inputFileStream.good() && j < numbersCount)
        {
            number = inputFileStream.get();
            numbers.push_back(number);
            //Print numbers on one line
            if(!MEASURE)
            {
                cout << number;
                if (j != numbersCount - 1) //we do not want to space after last number
                {
                    cout << " ";
                }
            }
            j++;
        }
        if(!MEASURE)
        {
            cout << endl;
        }

        /************* Parallern Enumeration sort algorithm ************/
        //Measure
        timespec timeStart;
        timespec timeEnd;
        if(MEASURE)
        {
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timeStart);
        }

        //Send data to CPUs [1]
        for(int i = 0; i < numbersCount; i++)
        {
            MPI_Send(&numbers[i], MSG_SIZE, MPI_INT, i + 1, TAGX, MPI_COMM_WORLD);
            MPI_Send(&numbers[i], MSG_SIZE, MPI_INT, FIRST_SLAVE, TAGY, MPI_COMM_WORLD);
        }

        //Receive data from slaves [6]
        int recievedData;
        for(int i = 0; i < numbersCount; i++)
        {
            MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, lastProcess, TAGO, MPI_COMM_WORLD, &status);
            numbers[numbersCount - i - 1] = recievedData;
        }

        //Measure
        if(MEASURE)
        {
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timeEnd);
            measureTime(timeStart, timeEnd);
        }

        //Print output
        if(!MEASURE)
        {
            for (int i = 0; i < numbers.size(); i++) {
                cout << numbers[i] << endl;
            }
        }
    }
    else //other processes CPU slaves, who make comparison
    {
        //Initialization of registers
        int C = INITC;
        int X = INIT;
        int Y = INIT;
        int Z = INIT;
        int recievedData;

        //Get X data from BUS [2]
        MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, MASTER, TAGX, MPI_COMM_WORLD, &status);
        X = recievedData;

        //Push Y values trought direct connection [3]
        for(int i = 0; i < numbersCount; i++)
        {
            MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, processId - 1, TAGY, MPI_COMM_WORLD, &status);
            Y = recievedData;
            if(processId < numbersCount) //last do not send
            {
                MPI_Send(&Y, MSG_SIZE, MPI_INT, processId + 1, TAGY, MPI_COMM_WORLD);
            }
            //Compare X and Y and increment C
            //Improvement!, now i can order same values, assuming http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.106.4976&rep=rep1&type=pdf
            if(i < processId - 1)
            {
                if(X >= Y)
                {
                    C++;
                }
            }
            else
            {
                if(X > Y)
                {
                    C++;
                }
            }
        }

        //Send Z to Cth process [4]
        MPI_Send(&X, MSG_SIZE, MPI_INT, C, TAGZ, MPI_COMM_WORLD);
        MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, TAGZ, MPI_COMM_WORLD, &status);
        Z = recievedData;

        //Push Z values trought direct connection to output [5]
        for(int i = 1; i <= processId + 1; i++)
        {
            if(processId == lastProcess) //last CPU send Z register to Output
            {
                MPI_Send(&Z, MSG_SIZE, MPI_INT, MASTER, TAGO, MPI_COMM_WORLD);
            }
            else //it is not last, so we send Z register to next neighbour
            {
                MPI_Send(&Z, MSG_SIZE, MPI_INT, processId + 1, TAGO, MPI_COMM_WORLD);
            }
            if(i < processId) //receive Z value from neighbours
            {
                MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, processId - 1, TAGO, MPI_COMM_WORLD, &status);
                Z = recievedData;
            }
        }
    }

    MPI_Finalize();
    return OK;
}