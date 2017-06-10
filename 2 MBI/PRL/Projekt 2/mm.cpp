/*
 * PRL Project2 2017 - Mesh Multiplication
 * Author: Filip Gulan (xgulan00)
 * Mail: xgulan00@stud.fit.vutbr.cz
 * Date: 27.4.2017
 */

#include <iostream>
#include <mpi.h>
#include <vector>
#include <string>
#include <fstream>
#include <time.h>
#include <sstream>

#define OK 0
#define FAILURE 1
#define MSG_SIZE 1
#define FIRST_PROCESS 0
#define TAG_R1 1
#define TAG_C1 2
#define TAG_R2 3
#define TAG_C2 4
#define TAG_SR 5
#define TAG_SC 6
#define TAG_CV 7
#define TAG_CH 7
#define TAG_O 8
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
 * Get matrix from file
 * @param fileName of matrix values
 * @param rows of matrix
 * @param cols of matrix
 * @return matrix
 */
vector<int> getMatrix(string fileName, int *rows, int *cols)
{
    vector<int> matrix;
    string line;
    int number;
    ifstream matrixFile (fileName.c_str());

    getline (matrixFile, line);
    *rows = stoi(line);

    while ( getline (matrixFile, line) ) //read by line
    {
        istringstream lineStream(line);
        while (lineStream >> number) //read by character
        {
            matrix.push_back(number);
        }
    }

    *cols = matrix.size() / *rows;
    matrixFile.close();
    return matrix;
}

/**
 * Get I index
 * @param cols
 * @param id
 * @return I index
 */
int getIIndex(int cols, int id)
{
    return id/cols;
}

/**
 * Get J index
 * @param cols
 * @param id
 * @return J index
 */
int getJIndex(int cols, int id)
{
    return id%cols;
}

/**
 * Get proces id, it is map function, map 2d matrix coordinates to 1d vector
 * @param i
 * @param j
 * @param cols
 * @return 1d index to vector
 */
int getProcessId(int i, int j, int cols)
{
    return (i * cols) + j;
}

/**
 * Equivalent for getProcessId
 * @param i
 * @param j
 * @param cols
 * @return
 */
int getMatrixIndex(int i, int j, int cols)
{
    return (i * cols) + j;
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
    int processId;

    MPI_Init(&argc, &argv); //initialize MPI
    MPI_Comm_size(MPI_COMM_WORLD,&processesCount); //get number of processes
    MPI_Comm_rank(MPI_COMM_WORLD,&processId); //get actual process id
    MPI_Status status;

    vector<int> mat1;
    vector<int> mat2;
    vector<int> matMul;
    vector<int> row;
    vector<int> col;
    int mat1Rows = 0;
    int mat1Cols = 0;
    int mat2Rows = 0;
    int mat2Cols = 0;
    int recievedData = 0;

    /************************** Step 1 - Send dimensions to all processes ****************************/
    if(processId == FIRST_PROCESS)
    {
        //Get matrix data
        mat1 = getMatrix("mat1", &mat1Rows, &mat1Cols);
        mat2 = getMatrix("mat2", &mat2Cols, &mat2Rows);
        for(int i = 1; i < processesCount; i++) //Distribute dimensions
        {
            MPI_Send(&mat1Rows, MSG_SIZE, MPI_INT, i, TAG_R1, MPI_COMM_WORLD);
            MPI_Send(&mat1Cols, MSG_SIZE, MPI_INT, i, TAG_C1, MPI_COMM_WORLD);
            MPI_Send(&mat2Rows, MSG_SIZE, MPI_INT, i, TAG_R2, MPI_COMM_WORLD);
            MPI_Send(&mat2Cols, MSG_SIZE, MPI_INT, i, TAG_C2, MPI_COMM_WORLD);
        }
    }
    else
    {
        //Get dimensions from first procesor
        MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, FIRST_PROCESS, TAG_R1, MPI_COMM_WORLD, &status);
        mat1Rows = recievedData;
        MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, FIRST_PROCESS, TAG_C1, MPI_COMM_WORLD, &status);
        mat1Cols = recievedData;
        MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, FIRST_PROCESS, TAG_R2, MPI_COMM_WORLD, &status);
        mat2Rows = recievedData;
        MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, FIRST_PROCESS, TAG_C2, MPI_COMM_WORLD, &status);
        mat2Cols = recievedData;
    }

    /************************** Step 2 - Compute i and j index of process *********************************/
    int iIndex = getIIndex(mat2Cols, processId);
    int jIndex = getJIndex(mat2Cols, processId);

    /************************* Step 3 - Distribute rows and cols to first processes **********************/
    if(processId == FIRST_PROCESS)
    {
        //Send to processes on first column
        for(int i = 0; i < mat1Rows; i++)
        {
            for(int j = 0; j < mat1Cols; j++)
            {
                MPI_Send(&mat1[getMatrixIndex(i, j, mat1Cols)], MSG_SIZE, MPI_INT, getProcessId(i, 0, mat2Cols), TAG_SR, MPI_COMM_WORLD);
            }
        }
        //Send to processes on first row
        for(int j = 0; j < mat2Cols; j++)
        {
            for(int i = 0; i < mat2Rows; i++)
            {
                MPI_Send(&mat2[getMatrixIndex(i, j, mat2Cols)], MSG_SIZE, MPI_INT, getProcessId(0, j, mat2Cols), TAG_SC, MPI_COMM_WORLD);
            }
        }
    }

    if(iIndex == 0) //process on first row
    {
        for(int i = 0; i < mat2Rows; i++)
        {
            MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, FIRST_PROCESS, TAG_SC, MPI_COMM_WORLD, &status);
            int number = recievedData;
            col.push_back(number);
        }
    }
    if(jIndex == 0) //process on first column
    {
        for(int j = 0; j < mat1Cols; j++)
        {
            MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, FIRST_PROCESS, TAG_SR, MPI_COMM_WORLD, &status);
            int number = recievedData;
            row.push_back(number);
        }
    }

    /********************* Step 5 - Algorithm Computation ***************************/
    timespec timeStart;
    timespec timeEnd;

    MPI_Barrier(MPI_COMM_WORLD); //because of time measuring
    if(processId == FIRST_PROCESS)
    {
        if(MEASURE)
        {
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timeStart);
        }
    }

    int X = 0; //top
    int Y = 0; //left
    int C = 0;
    for(int i = 0; i < mat1Cols; i++) //for mat1rows or mat2cols it is the same
    {
        if(iIndex != 0) //if it is not first in row
        {
            MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, getProcessId(iIndex - 1, jIndex, mat2Cols), TAG_CV, MPI_COMM_WORLD, &status);
            X = recievedData;
        }
        else //it is first in row, we get value from internal vector
        {
            X = col[0];
            col.erase(col.begin());
        }

        if(jIndex != 0) //if it is not first in col
        {
            MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, getProcessId(iIndex, jIndex - 1, mat2Cols), TAG_CH, MPI_COMM_WORLD, &status);
            Y = recievedData;
        }
        else //it is first in col, we get value from internal vector
        {
            Y = row[0];
            row.erase(row.begin());
        }

        C = C + (X * Y); //computation

        if(iIndex < mat1Rows - 1) //it is not last in row
        {
            MPI_Send(&X, MSG_SIZE, MPI_INT, getProcessId(iIndex + 1, jIndex, mat2Cols), TAG_CV, MPI_COMM_WORLD);
        }
        if(jIndex < mat2Cols - 1) //it is not last in coll
        {
            MPI_Send(&Y, MSG_SIZE, MPI_INT, getProcessId(iIndex, jIndex + 1, mat2Cols), TAG_CH, MPI_COMM_WORLD);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD); //because of time measuring
    if(processId == FIRST_PROCESS)
    {
        if(MEASURE)
        {
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &timeEnd);
            measureTime(timeStart, timeEnd);
        }
    }

    /*************************** Step 6 - Get data from processes ******************************************/
    if(processId == FIRST_PROCESS) //get values from processors to processor 0
    {
        matMul.push_back(C);
        for(int i = 1; i < processesCount; i++) //get data
        {
            MPI_Recv(&recievedData, MSG_SIZE, MPI_INT, i, TAG_O, MPI_COMM_WORLD, &status);
            matMul.push_back(recievedData);
        }

        if(!MEASURE) //if we do not measure, we print result on stdout
        {
            cout << mat1Rows << ":" << mat2Cols << endl;
            for (int i = 0; i < matMul.size(); i++)
            {
                cout << matMul[i];
                if (((i + 1) % mat2Cols) == 0)
                {
                    cout << endl;
                } else
                {
                    cout << " ";
                }
            }
        }
    }
    else //send values from processors to processor 0
    {
        MPI_Send(&C, MSG_SIZE, MPI_INT, FIRST_PROCESS, TAG_O, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return OK;
}