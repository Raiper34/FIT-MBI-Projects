/**
 * @file        proj02.cpp
 * @author      Jiri Jaros, Radek Hrbacek, Filip Vaverka and Vojtech Nikl\n
 *              Faculty of Information Technology\n
 *              Brno University of Technology\n
 *              jarosjir@fit.vutbr.cz
 *
 * @brief       Parallelisation of Heat Distribution Method in Heterogenous
 *              Media using MPI and OpenMP
 *
 * @version     2017
 * @date        10 April 2015, 10:22 (created)\n
 *              28 March 2017, 12:02 (revised)
 *
 * @detail
 * This is the main file of the project. Add all code here.
 */


#include <mpi.h>
#include <omp.h>

#include <string.h>
#include <string>
#include <cmath>

#include <hdf5.h>

#include <sstream>
#include <immintrin.h>

#include "MaterialProperties.h"
#include "BasicRoutines.h"


using namespace std;


//----------------------------------------------------------------------------//
//---------------------------- Global variables ------------------------------//
//----------------------------------------------------------------------------//

/// Temperature data for sequential version.
float *seqResult = NULL;
/// Temperature data for parallel method.
float *parResult = NULL;

/// Parameters of the simulation
TParameters parameters;

/// Material properties
TMaterialProperties materialProperties;


//----------------------------------------------------------------------------//
//------------------------- Function declarations ----------------------------//
//----------------------------------------------------------------------------//

/// Sequential implementation of the Heat distribution
void SequentialHeatDistribution(float                     *seqResult,
                                const TMaterialProperties &materialProperties,
                                const TParameters         &parameters,
                                string                     outputFileName);

/// Parallel Implementation of the Heat distribution (Non-overlapped file output)
void ParallelHeatDistribution(float                     *parResult,
                              const TMaterialProperties &materialProperties,
                              const TParameters         &parameters,
                              string                     outputFileName);

/// Store time step into output file
void StoreDataIntoFile(hid_t         h5fileId,
                       const float * data,
                       const size_t  edgeSize,
                       const size_t  snapshotId,
                       const size_t  iteration);

/// Store time step into output file using parallel HDF5
void StoreDataIntoFileParallel(hid_t h5fileId,
                               const float *data,
                               const size_t edgeSize,
                               const size_t tileWidth, const size_t tileHeight,
                               const size_t tilePosX, const size_t tilePosY,
                               const size_t snapshotId,
                               const size_t iteration);


//----------------------------------------------------------------------------//
//------------------------- Function implementation  -------------------------//
//----------------------------------------------------------------------------//


void ComputePoint(float  *oldTemp,
                  float  *newTemp,
                  float  *params,
                  int    *map,
                  size_t  i,
                  size_t  j,
                  size_t  edgeSize,
                  float   airFlowRate,
                  float   coolerTemp)
{
    // [i] Calculate neighbor indices
    const int center    = i * edgeSize + j;
    const int top[2]    = { center - (int)edgeSize, center - 2*(int)edgeSize };
    const int bottom[2] = { center + (int)edgeSize, center + 2*(int)edgeSize };
    const int left[2]   = { center - 1, center - 2};
    const int right[2]  = { center + 1, center + 2};

    // [ii] The reciprocal value of the sum of domain parameters for normalization
    const float frac = 1.0f / (params[top[0]]    + params[top[1]]    +
                               params[bottom[0]] + params[bottom[1]] +
                               params[left[0]]   + params[left[1]]   +
                               params[right[0]]  + params[right[1]]  +
                               params[center]);

    // [iii] Calculate new temperature in the grid point
    float pointTemp =
            oldTemp[top[0]]    * params[top[0]]    * frac +
            oldTemp[top[1]]    * params[top[1]]    * frac +
            oldTemp[bottom[0]] * params[bottom[0]] * frac +
            oldTemp[bottom[1]] * params[bottom[1]] * frac +
            oldTemp[left[0]]   * params[left[0]]   * frac +
            oldTemp[left[1]]   * params[left[1]]   * frac +
            oldTemp[right[0]]  * params[right[0]]  * frac +
            oldTemp[right[1]]  * params[right[1]]  * frac +
            oldTemp[center]    * params[center]    * frac;

    // [iv] Remove some of the heat due to air flow (5% of the new air)
    pointTemp = (map[center] == 0)
                ? (airFlowRate * coolerTemp) + ((1.0f - airFlowRate) * pointTemp)
                : pointTemp;

    newTemp[center] = pointTemp;
}

/**
 * Sequential version of the Heat distribution in heterogenous 2D medium
 * @param [out] seqResult          - Final heat distribution
 * @param [in]  materialProperties - Material properties
 * @param [in]  parameters         - parameters of the simulation
 * @param [in]  outputFileName     - Output file name (if NULL string, do not store)
 *
 */
void SequentialHeatDistribution(float                      *seqResult,
                                const TMaterialProperties &materialProperties,
                                const TParameters         &parameters,
                                string                     outputFileName)
{
    // [1] Create a new output hdf5 file
    hid_t file_id = H5I_INVALID_HID;

    if (outputFileName != "")
    {
        if (outputFileName.find(".h5") == string::npos)
            outputFileName.append("_seq.h5");
        else
            outputFileName.insert(outputFileName.find_last_of("."), "_seq");

        file_id = H5Fcreate(outputFileName.c_str(),
                            H5F_ACC_TRUNC,
                            H5P_DEFAULT,
                            H5P_DEFAULT);
        if (file_id < 0) ios::failure("Cannot create output file");
    }


    // [2] A temporary array is needed to prevent mixing of data form step t and t+1
    float *tempArray = (float *)_mm_malloc(materialProperties.nGridPoints *
                                           sizeof(float), DATA_ALIGNMENT);

    // [3] Init arrays
    for (size_t i = 0; i < materialProperties.nGridPoints; i++)
    {
        tempArray[i] = materialProperties.initTemp[i];
        seqResult[i] = materialProperties.initTemp[i];
    }

    // [4] t+1 values, t values
    float *newTemp = seqResult;
    float *oldTemp = tempArray;

    if (!parameters.batchMode)
        printf("Starting sequential simulation... \n");

    //---------------------- [5] start the stop watch ------------------------------//
    double elapsedTime = MPI_Wtime();
    size_t i, j;
    size_t iteration;
    float middleColAvgTemp = 0.0f;
    size_t printCounter = 1;

    // [6] Start the iterative simulation
    for (iteration = 0; iteration < parameters.nIterations; iteration++)
    {
        // [a] calculate one iteration of the heat distribution (skip the grid points at the edges)
        for (i = 2; i < materialProperties.edgeSize - 2; i++)
            for (j = 2; j < materialProperties.edgeSize - 2; j++)
                ComputePoint(oldTemp,
                             newTemp,
                             materialProperties.domainParams,
                             materialProperties.domainMap,
                             i, j,
                             materialProperties.edgeSize,
                             parameters.airFlowRate,
                             materialProperties.coolerTemp);

        // [b] Compute the average temperature in the middle column
        middleColAvgTemp = 0.0f;
        for (i = 0; i < materialProperties.edgeSize; i++)
            middleColAvgTemp += newTemp[i*materialProperties.edgeSize +
                                        materialProperties.edgeSize/2];
        middleColAvgTemp /= materialProperties.edgeSize;

        // [c] Store time step in the output file if necessary
        if ((file_id != H5I_INVALID_HID)  && ((iteration % parameters.diskWriteIntensity) == 0))
        {

            StoreDataIntoFile(file_id,
                              newTemp,
                              materialProperties.edgeSize,
                              iteration / parameters.diskWriteIntensity,
                              iteration);
        }

        // [d] Swap new and old values
        swap(newTemp, oldTemp);

        // [e] Print progress and average temperature of the middle column
        if ( ((float)(iteration) >= (parameters.nIterations-1) / 10.0f * (float)printCounter)
             && !parameters.batchMode)
        {
            printf("Progress %ld%% (Average Temperature %.2f degrees)\n",
                   (iteration+1) * 100L / (parameters.nIterations),
                   middleColAvgTemp);
            ++printCounter;
        }

        /*printf("Global sequential array is:\n");
        for (int y = 0; y < materialProperties.edgeSize; y++)
        {
            for (int x = 0; x < materialProperties.edgeSize; x++)
            {
                printf("%f ", seqResult[y * materialProperties.edgeSize + x]);
            }
            printf("\n");
        }*/
    } // for iteration

    //-------------------- stop the stop watch  --------------------------------//
    double totalTime = MPI_Wtime() - elapsedTime;

    // [7] Print final result
    if (!parameters.batchMode)
        printf("\nExecution time of sequential version %.5f\n", totalTime);
    else
        printf("%s;%s;%f;%e;%e\n", outputFileName.c_str(), "seq",
               middleColAvgTemp, totalTime,
               totalTime / parameters.nIterations);

    // Close the output file
    if (file_id != H5I_INVALID_HID) H5Fclose(file_id);

    // [8] Return correct results in the correct array
    if (iteration & 1)
        memcpy(seqResult, tempArray, materialProperties.nGridPoints * sizeof(float));

    /*printf("Global sequential array is:\n");
    for (int y = 0; y < materialProperties.edgeSize; y++)
    {
        for (int x = 0; x < materialProperties.edgeSize; x++)
        {
            printf("%f ", seqResult[y * materialProperties.edgeSize + x]);
        }
        printf("\n");
    }*/

    _mm_free(tempArray);
} // end of SequentialHeatDistribution
//------------------------------------------------------------------------------


/**
 * Parallel version of the Heat distribution in heterogenous 2D medium
 * @param [out] parResult          - Final heat distribution
 * @param [in]  materialProperties - Material properties
 * @param [in]  parameters         - parameters of the simulation
 * @param [in]  outputFileName     - Output file name (if NULL string, do not store)
 *
 * @note This is the function that students should implement.
 */
void ParallelHeatDistribution(float                     *parResult,
                              const TMaterialProperties &materialProperties,
                              const TParameters         &parameters,
                              string                     outputFileName)
{
#define HALOZONE 4
#define TAG_LEFT 0 //from left to right
#define TAG_RIGHT 1 //from right to left
#define TAG_TOP 2 //from top to bottom
#define TAG_BOTTOM 3 //from bottom to top

    // Get MPI rank and size
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Status status;

    /************************************** Open file for writing ********************************************/
    hid_t file_id = H5I_INVALID_HID;
    if(!parameters.useParallelIO) //open with only one process
    {
        // Serial I/O
        if(rank == 0 && outputFileName != "")
        {
            if(outputFileName.find(".h5") == string::npos)
                outputFileName.append("_par.h5");
            else
                outputFileName.insert(outputFileName.find_last_of("."), "_par");

            file_id = H5Fcreate(outputFileName.c_str(),
                                H5F_ACC_TRUNC,
                                H5P_DEFAULT,
                                H5P_DEFAULT);
            if(file_id < 0) ios::failure("Cannot create output file");
        }
    }
    else //open with all processes
    {
        // Parallel I/O
        if(outputFileName != "")
        {
            if(outputFileName.find(".h5") == string::npos)
                outputFileName.append("_par.h5");
            else
                outputFileName.insert(outputFileName.find_last_of("."), "_par");

            hid_t hPropList = H5Pcreate(H5P_FILE_ACCESS);
            H5Pset_fapl_mpio(hPropList, MPI_COMM_WORLD, MPI_INFO_NULL);

            file_id = H5Fcreate(outputFileName.c_str(),
                                H5F_ACC_TRUNC,
                                H5P_DEFAULT,
                                hPropList);
            H5Pclose(hPropList);
            if(file_id < 0) ios::failure("Cannot create output file");
        }
    }
    /*****************************************************************************************************/

    size_t dimension = materialProperties.edgeSize; //todo remove ///////////////////////////////////////////////////

    float *domainParamsTemp;
    float *domainMapTemp;
    if(rank == 0)
    {
        domainParamsTemp = (float *) _mm_malloc(dimension * dimension * sizeof(float), DATA_ALIGNMENT);
        domainMapTemp = (float *) _mm_malloc(dimension * dimension * sizeof(float), DATA_ALIGNMENT);
        for (size_t i = 0; i < dimension * dimension; i++) {
            parResult[i] = materialProperties.initTemp[i];
            domainParamsTemp[i] = materialProperties.domainParams[i];
            domainMapTemp[i] = materialProperties.domainMap[i];
        }
        if (!parameters.batchMode)
            printf("Starting parallel simulation... \n");
    }

    //Dimensions
    size_t tileWidth = 0;
    size_t tileHeight = 0;
    if(sqrt(size) == floor(sqrt(size))) { //even power (suda,parna)
        tileHeight = dimension/sqrt(size);
        tileWidth = dimension/sqrt(size);
    } else { //odd power (licha neparna)
        tileHeight = dimension/(2*sqrt(size/2));
        tileWidth = dimension/((sqrt(size/2)));
    }

    //Indexes
    int cols = dimension/(tileWidth);
    int rows = dimension/(tileHeight);
    int iIndex = rank/cols;
    int jIndex = rank%cols;

    //Tiles arrays
    float *newTile = (float *) malloc((tileWidth + HALOZONE) * (tileHeight + HALOZONE) * sizeof(float));
    float *oldTile = (float *) malloc((tileWidth + HALOZONE) * (tileHeight + HALOZONE) * sizeof(float));
    float *domainParamsTile = (float *) malloc((tileWidth + HALOZONE) * (tileHeight + HALOZONE) * sizeof(float));
    int *domainMapTile = (int *) malloc((tileWidth + HALOZONE) * (tileHeight + HALOZONE) * sizeof(int));
    for(int i = 0; i < (tileWidth + HALOZONE) * (tileHeight + HALOZONE); i++)
    {
        newTile[i] = -1000; //todo
        oldTile[i] = -1000;
        domainParamsTile[i] = -1000;
        domainMapTile[i] = -1000;
    }

    /********** Scaterv ********************/
    //Tile with halos
    int dimensions1[2] = {dimension, dimension}; //of whole grid
    int tileDimensions1[2] = {tileHeight, tileWidth}; //of tile
    int tileStart1[2] = {0,0}; //where tile start
    MPI_Datatype type1, haloTileType;
    MPI_Type_create_subarray(2, dimensions1, tileDimensions1, tileStart1, MPI_ORDER_C, MPI_FLOAT, &type1);
    MPI_Type_create_resized(type1, 0, sizeof(float), &haloTileType);
    MPI_Type_commit(&haloTileType);

    //Tile without halos
    int dimensions2[2] = {tileHeight + HALOZONE, tileWidth + HALOZONE}; //of whole grid
    int tileDimensions2[2] = {tileHeight, tileWidth}; //of tile
    int tileStart2[2] = {0,0}; //where tile start
    MPI_Datatype type2, tileType;
    MPI_Type_create_subarray(2, dimensions2, tileDimensions2, tileStart2, MPI_ORDER_C, MPI_FLOAT, &type2);
    MPI_Type_create_resized(type2, 0, sizeof(float), &tileType);
    MPI_Type_commit(&tileType);

    float *dataPtr = NULL;
    float *domainParamsPtr = NULL;
    float *domainMapPtr = NULL;
    if (rank == 0)
    {
        dataPtr = &(parResult[0]); //adress on start of grid
        domainParamsPtr = &(domainParamsTemp[0]);
        domainMapPtr = &(domainMapTemp[0]);
    }

    int sendcounts[size];
    int displs[size];

    if (rank == 0)
    {
        for (int i = 0; i < size; i++)
        {
            sendcounts[i] = 1; //how much data send to processes, only one data of subArrType
        }
        int offset = 0;
        for (int i = 0 ; i < rows; i++)
        {
            for (int j = 0; j < cols; j++)
            {
                displs[i * cols + j] = (tileHeight * tileWidth * cols * i) + (tileWidth * j);
            }
        }
    }

    MPI_Scatterv(dataPtr, sendcounts, displs, haloTileType, &(oldTile[2 * (tileWidth + HALOZONE) + 2]), 1, tileType, 0, MPI_COMM_WORLD);
    MPI_Scatterv(domainParamsPtr, sendcounts, displs, haloTileType, &(domainParamsTile[2 * (tileWidth + HALOZONE) + 2]), 1, tileType, 0, MPI_COMM_WORLD);
    MPI_Scatterv(domainMapPtr, sendcounts, displs, haloTileType, &(domainMapTile[2 * (tileWidth + HALOZONE) + 2]), 1, tileType, 0, MPI_COMM_WORLD);
    /***************************************/

    /*********** Send halozones ************/
    //Vertical halozone
    int dimensions3[2] = {tileHeight + HALOZONE, tileWidth + HALOZONE}; //of whole grid
    int tileDimensions3[2] = {tileHeight, 2}; //of tile
    int tileStart3[2] = {0,0}; //where tile start
    MPI_Datatype type3, horizontalHaloType;
    MPI_Type_create_subarray(2, dimensions3, tileDimensions3, tileStart3, MPI_ORDER_C, MPI_FLOAT, &type3);
    MPI_Type_create_resized(type3, 0, sizeof(float), &horizontalHaloType);
    MPI_Type_commit(&horizontalHaloType);

    //Horizontal halozone
    int dimensions4[2] = {tileHeight + HALOZONE, tileWidth + HALOZONE}; //of whole grid
    int tileDimensions4[2] = {2, tileWidth}; //of tile
    int tileStart4[2] = {0,0}; //where tile start
    MPI_Datatype type4, verticalHaloType;
    MPI_Type_create_subarray(2, dimensions4, tileDimensions4, tileStart4, MPI_ORDER_C, MPI_FLOAT, &type4);
    MPI_Type_create_resized(type4, 0, sizeof(float), &verticalHaloType);
    MPI_Type_commit(&verticalHaloType);

    //Left halozone
    if(jIndex != cols - 1)
    {
        MPI_Send(&(oldTile[2 * (tileWidth + HALOZONE) + tileWidth]), 1, horizontalHaloType, (iIndex * cols) + (jIndex + 1), TAG_LEFT, MPI_COMM_WORLD);
        MPI_Send(&(domainParamsTile[2 * (tileWidth + HALOZONE) + tileWidth]), 1, horizontalHaloType, (iIndex * cols) + (jIndex + 1), TAG_LEFT + 10, MPI_COMM_WORLD);
        MPI_Send(&(domainMapTile[2 * (tileWidth + HALOZONE) + tileWidth]), 1, horizontalHaloType, (iIndex * cols) + (jIndex + 1), TAG_LEFT + 20, MPI_COMM_WORLD);
    }
    if(jIndex != 0)
    {
        MPI_Recv(&(oldTile[2 * (tileWidth + HALOZONE) + 0]), 1, horizontalHaloType, (iIndex * cols) + (jIndex - 1), TAG_LEFT, MPI_COMM_WORLD, &status);
        MPI_Recv(&(domainParamsTile[2 * (tileWidth + HALOZONE) + 0]), 1, horizontalHaloType, (iIndex * cols) + (jIndex - 1), TAG_LEFT + 10, MPI_COMM_WORLD, &status);
        MPI_Recv(&(domainMapTile[2 * (tileWidth + HALOZONE) + 0]), 1, horizontalHaloType, (iIndex * cols) + (jIndex - 1), TAG_LEFT + 20, MPI_COMM_WORLD, &status);
    }
    //Right halozone
    if(jIndex != 0)
    {
        MPI_Send(&(oldTile[2 * (tileWidth + HALOZONE) + 2]), 1, horizontalHaloType, (iIndex * cols) + (jIndex - 1), TAG_RIGHT, MPI_COMM_WORLD);
        MPI_Send(&(domainParamsTile[2 * (tileWidth + HALOZONE) + 2]), 1, horizontalHaloType, (iIndex * cols) + (jIndex - 1), TAG_RIGHT + 10, MPI_COMM_WORLD);
        MPI_Send(&(domainMapTile[2 * (tileWidth + HALOZONE) + 2]), 1, horizontalHaloType, (iIndex * cols) + (jIndex - 1), TAG_RIGHT + 20, MPI_COMM_WORLD);
    }
    if(jIndex != cols - 1)
    {
        MPI_Recv(&(oldTile[2 * (tileWidth + HALOZONE) + tileWidth + 2]), 1, horizontalHaloType, (iIndex * cols) + (jIndex + 1), TAG_RIGHT, MPI_COMM_WORLD, &status);
        MPI_Recv(&(domainParamsTile[2 * (tileWidth + HALOZONE) + tileWidth + 2]), 1, horizontalHaloType, (iIndex * cols) + (jIndex + 1), TAG_RIGHT + 10, MPI_COMM_WORLD, &status);
        MPI_Recv(&(domainMapTile[2 * (tileWidth + HALOZONE) + tileWidth + 2]), 1, horizontalHaloType, (iIndex * cols) + (jIndex + 1), TAG_RIGHT + 20, MPI_COMM_WORLD, &status);
    }
    //Top halozone
    if(iIndex != rows - 1)
    {
        MPI_Send(&(oldTile[(tileHeight) * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex + 1) * cols) + (jIndex), TAG_TOP, MPI_COMM_WORLD);
        MPI_Send(&(domainParamsTile[(tileHeight) * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex + 1) * cols) + (jIndex), TAG_TOP + 10, MPI_COMM_WORLD);
        MPI_Send(&(domainMapTile[(tileHeight) * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex + 1) * cols) + (jIndex), TAG_TOP + 20, MPI_COMM_WORLD);
    }
    if(iIndex != 0)
    {
        MPI_Recv(&(oldTile[0 * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex - 1) * cols) + (jIndex), TAG_TOP, MPI_COMM_WORLD, &status);
        MPI_Recv(&(domainParamsTile[0 * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex - 1) * cols) + (jIndex), TAG_TOP + 10, MPI_COMM_WORLD, &status);
        MPI_Recv(&(domainMapTile[0 * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex - 1) * cols) + (jIndex), TAG_TOP + 20, MPI_COMM_WORLD, &status);
    }
    //Bottom halozone
    if(iIndex != 0)
    {
        MPI_Send(&(oldTile[2 * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex - 1) * cols) + (jIndex), TAG_BOTTOM, MPI_COMM_WORLD);
        MPI_Send(&(domainParamsTile[2 * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex - 1) * cols) + (jIndex), TAG_BOTTOM + 10, MPI_COMM_WORLD);
        MPI_Send(&(domainMapTile[2 * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex - 1) * cols) + (jIndex), TAG_BOTTOM + 20, MPI_COMM_WORLD);
    }
    if(iIndex != rows - 1)
    {
        MPI_Recv(&(oldTile[(tileHeight + 2) * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex + 1) * cols) + (jIndex), TAG_BOTTOM, MPI_COMM_WORLD, &status);
        MPI_Recv(&(domainParamsTile[(tileHeight + 2) * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex + 1) * cols) + (jIndex), TAG_BOTTOM + 10, MPI_COMM_WORLD, &status);
        MPI_Recv(&(domainMapTile[(tileHeight + 2) * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex + 1) * cols) + (jIndex), TAG_BOTTOM + 20, MPI_COMM_WORLD, &status);
    }

    //Copy oldTile to newTile
    for(int i = 0; i < (tileWidth + 4) * (tileHeight + 4); i++)
    {
        newTile[i] = oldTile[i];
    }

    //Left halozone
    /*if(jIndex != cols - 1)
    {
        MPI_Send(&(newTile[2 * (tileWidth + HALOZONE) + tileWidth]), 1, horizontalHaloType, (iIndex * cols) + (jIndex + 1), TAG_LEFT, MPI_COMM_WORLD);
    }
    //Right halozone
    if(jIndex != 0)
    {
        MPI_Send(&(newTile[2 * (tileWidth + HALOZONE) + 2]), 1, horizontalHaloType, (iIndex * cols) + (jIndex - 1), TAG_RIGHT, MPI_COMM_WORLD);
    }
    //Top halozone
    if(iIndex != rows - 1)
    {
        MPI_Send(&(newTile[(tileHeight) * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex + 1) * cols) + (jIndex), TAG_TOP, MPI_COMM_WORLD);
    }
    //Bottom halozone
    if(iIndex != 0)
    {
        MPI_Send(&(newTile[2 * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex - 1) * cols) + (jIndex), TAG_BOTTOM, MPI_COMM_WORLD);
    }*/

    /**********************************************/

    /******************* Computation ********************/
    double elapsedTime;
    if(rank == 0)
    {
        elapsedTime = MPI_Wtime();
    }

    float middleColAvgTemp = 0.0f;
    float tileMiddleColAvgTemp = 0.0f;
    size_t printCounter = 1;

    //For indexes
    int iStart = (iIndex == 0)? 4 : 2;
    int iEnd = (iIndex == rows - 1)? tileHeight : tileHeight + 2;
    int jStart = (jIndex == 0)? 4 : 2;
    int jEnd = (jIndex == cols - 1)? tileWidth : tileWidth + 2;

    MPI_Request requests[8];
    MPI_Status statuses[8];
    int counter = 0;
    MPI_Request requests2[8];
    MPI_Status statuses2[8];
    int counter2 = 0;
    int iteration;
    for (iteration = 0; iteration < parameters.nIterations - 1; iteration++) //todo parameters.nIterations
    { //todo start
        counter = 0;
        counter2 = 0;
        //Left halozone
        if(jIndex != 0)
        {
            MPI_Irecv(&(newTile[2 * (tileWidth + HALOZONE) + 0]), 1, horizontalHaloType, (iIndex * cols) + (jIndex - 1), TAG_LEFT, MPI_COMM_WORLD, &requests[counter]);
            counter++;
        }
        //Right halozone
        if(jIndex != cols - 1)
        {
            MPI_Irecv(&(newTile[2 * (tileWidth + HALOZONE) + tileWidth + 2]), 1, horizontalHaloType, (iIndex * cols) + (jIndex + 1), TAG_RIGHT, MPI_COMM_WORLD, &requests[counter]);
            counter++;
        }
        //Top halozone
        if(iIndex != 0)
        {
            MPI_Irecv(&(newTile[0 * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex - 1) * cols) + (jIndex), TAG_TOP, MPI_COMM_WORLD, &requests[counter]);
            counter++;
        }
        //Bottom halozone
        if(iIndex != rows - 1)
        {
            MPI_Irecv(&(newTile[(tileHeight + 2) * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex + 1) * cols) + (jIndex), TAG_BOTTOM, MPI_COMM_WORLD, &requests[counter]);
            counter++;
        }
        //MPI_Waitall(counter, requests, statuses);


        /*for (int i = iStart ; i < iEnd; i++)
        {
            for (int j = jStart; j < jEnd; j++)
            {
                ComputePoint(oldTile,
                             newTile,
                             domainParamsTile,
                             (int*)domainMapTile,
                             i, j,
                             (tileWidth + HALOZONE),
                             parameters.airFlowRate,
                             materialProperties.coolerTemp);
            }
        }*/

        //Left halozone
        if(jIndex != cols - 1)
        {
            for (int i = iStart; i < iEnd; i++)
            {
                for (int j = tileWidth; j < tileWidth + 2; j++)
                {
                    ComputePoint(oldTile,
                                 newTile,
                                 domainParamsTile,
                                 domainMapTile,
                                 i, j,
                                 (tileWidth + HALOZONE),
                                 parameters.airFlowRate,
                                 materialProperties.coolerTemp);
                }
            }

        }
        //Right halozone
        if(jIndex != 0)
        {
            for (int i = iStart; i < iEnd; i++)
            {
                for (int j = 2; j < 4; j++)
                {
                    ComputePoint(oldTile,
                                 newTile,
                                 domainParamsTile,
                                 domainMapTile,
                                 i, j,
                                 (tileWidth + HALOZONE),
                                 parameters.airFlowRate,
                                 materialProperties.coolerTemp);
                }
            }

        }
        //Top halozone
        if(iIndex != rows - 1)
        {
            for (int i = tileHeight; i < tileHeight + 2; ++i)
            {
                for (int j = jStart; j < jEnd; ++j)
                {
                    ComputePoint(oldTile,
                                 newTile,
                                 domainParamsTile,
                                 domainMapTile,
                                 i, j,
                                 (tileWidth + HALOZONE),
                                 parameters.airFlowRate,
                                 materialProperties.coolerTemp);
                }

            }
        }
        //Bottom halozone
        if(iIndex != 0)
        {
            for (int i = 2; i < 4; i++)
            {
                for (int j = jStart; j < jEnd; j++)
                {
                    ComputePoint(oldTile,
                                 newTile,
                                 domainParamsTile,
                                 domainMapTile,
                                 i, j,
                                 (tileWidth + HALOZONE),
                                 parameters.airFlowRate,
                                 materialProperties.coolerTemp);
                }

            }
        }

        //Left halozone
        if(jIndex != cols - 1)
        {
            MPI_Isend(&(newTile[2 * (tileWidth + HALOZONE) + tileWidth]), 1, horizontalHaloType, (iIndex * cols) + (jIndex + 1), TAG_LEFT, MPI_COMM_WORLD, &requests[counter]);
            counter++;
        }
        //Right halozone
        if(jIndex != 0)
        {
            MPI_Isend(&(newTile[2 * (tileWidth + HALOZONE) + 2]), 1, horizontalHaloType, (iIndex * cols) + (jIndex - 1), TAG_RIGHT, MPI_COMM_WORLD, &requests[counter]);
            counter++;
        }
        //Top halozone
        if(iIndex != rows - 1)
        {
            MPI_Isend(&(newTile[(tileHeight) * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex + 1) * cols) + (jIndex), TAG_TOP, MPI_COMM_WORLD, &requests[counter]);
            counter++;
        }
        //Bottom halozone
        if(iIndex != 0)
        {
            MPI_Isend(&(newTile[2 * (tileWidth + HALOZONE) + 2]), 1, verticalHaloType, ((iIndex - 1) * cols) + (jIndex), TAG_BOTTOM, MPI_COMM_WORLD, &requests[counter]);
            counter++;
        }
        //MPI_Waitall(counter2, requests2, statuses2);

        for (int i = 4 ; i < tileHeight; i++)
        {
            for (int j = 4; j < tileWidth; j++)
            {
                ComputePoint(oldTile,
                             newTile,
                             domainParamsTile,
                             domainMapTile,
                             i, j,
                             (tileWidth + HALOZONE),
                             parameters.airFlowRate,
                             materialProperties.coolerTemp);
            }
        }

        MPI_Waitall(counter, requests, statuses);


        /*for(int j = 0; j < size; j++)
        {
            if (rank == j)
            {
                printf("Local process on rank %d is:\n", rank);
                for (int i = 0 ; i < (tileHeight + HALOZONE); i++)
                {
                    for (int x = 0; x < (tileWidth + HALOZONE); x++)
                    {
                        printf("%d ", (int)oldTile[i * (tileWidth + HALOZONE) + x]);
                    }
                    printf("\n");
                }
            }
            MPI_Barrier(MPI_COMM_WORLD); //todo remove
        }*/

        /************** Middle column computation ************/
        if(rank == 0)
        {
            middleColAvgTemp = 0.0f;
        }

        tileMiddleColAvgTemp = 0.0f;
        if((jIndex == cols / 2) && jIndex == cols / 2)
        {
            for(int i = 2; i < tileHeight + 2; i++)
            {
                tileMiddleColAvgTemp += newTile[i * (tileWidth + HALOZONE) + ((size > 2) ? 2 : (2 + tileWidth/2))];
            }
        }
        MPI_Reduce(&tileMiddleColAvgTemp, &middleColAvgTemp, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0)
        {
            middleColAvgTemp /= dimension;
        }
        /****************************************************/


        /**************************** Write output *************************/
        if (iteration % parameters.diskWriteIntensity == 0)
        {
            if(!parameters.useParallelIO)
            {
                // Serial I/O
                // store data to root
                // *** Zde posbirejte data do 0. procesu, ktery vytvarel vystupni soubor ***
                MPI_Gatherv(&(newTile[2 * (tileWidth + HALOZONE) + 2]), 1,  tileType, dataPtr, sendcounts, displs, haloTileType, 0, MPI_COMM_WORLD); //todo really old?
                // store time step in the output file if necessary
                if (rank == 0 && file_id != H5I_INVALID_HID)
                {
                    StoreDataIntoFile(file_id,
                                      parResult,
                                      materialProperties.edgeSize,
                                      iteration / parameters.diskWriteIntensity,
                                      iteration);
                }
            }
            else
            {
                // Parallel I/O
                if(file_id != H5I_INVALID_HID)
                {
                    StoreDataIntoFileParallel(file_id,
                                              newTile,
                                              materialProperties.edgeSize,
                                              tileWidth + 4, tileHeight + 4,
                                              (tileWidth) * jIndex, (tileHeight) * iIndex,
                                              iteration / parameters.diskWriteIntensity, iteration);
                }
            }
        }
        /******************************************************************/

        swap(newTile, oldTile);

        if(rank == 0)
        {
            if (((float) (iteration) >= (parameters.nIterations - 2) / 10.0f * (float) printCounter) && !parameters.batchMode)
            {
                printf("Progress %ld%% (Average Temperature %.2f degrees)\n", (iteration + 1) * 100L / (parameters.nIterations - 1), middleColAvgTemp);
                ++printCounter;
            }
        }
        /*if(rank == 0)
        {
            printf("Global pararel array is:\n");
            for (int y = 0; y < materialProperties.edgeSize; y++)
            {
                for (int x = 0; x < materialProperties.edgeSize; x++)
                {
                    printf("%f ", parResult[y * materialProperties.edgeSize + x]);
                }
                printf("\n");
            }
        }*/
    } //todo end


    /****************************************************/

    double totalTime;
    if(rank == 0)
    {

        totalTime = MPI_Wtime() - elapsedTime;
        if (!parameters.batchMode)
            printf("\nExecution time of parallel version %.5f\n", totalTime);
        else
            printf("%s;%s;%f;%e;%e\n", outputFileName.c_str(), "seq",
                   middleColAvgTemp, totalTime,
                   totalTime / parameters.nIterations - 1);
    }

    // close the output file
    if (file_id != H5I_INVALID_HID) H5Fclose(file_id);

    MPI_Gatherv(&(oldTile[2 * (tileWidth + HALOZONE) + 2]), 1, tileType, dataPtr, sendcounts, displs, haloTileType, 0, MPI_COMM_WORLD);
    if(rank == 0)
    {
        /*printf("Global pararel array is:\n");
        for (int y = 0; y < materialProperties.edgeSize; y++)
        {
            for (int x = 0; x < materialProperties.edgeSize; x++)
            {
                printf("%f ", parResult[y * materialProperties.edgeSize + x]);
            }
            printf("\n");
        }*/

        _mm_free(domainParamsTemp);
        _mm_free(domainMapTemp);
    }
} // end of ParallelHeatDistribution
//------------------------------------------------------------------------------


/**
 * Store time step into output file (as a new dataset in Pixie format
 * @param [in] h5fileID   - handle to the output file
 * @param [in] data       - data to write
 * @param [in] edgeSize   - size of the domain
 * @param [in] snapshotId - snapshot id
 * @param [in] iteration  - id of iteration);
 */
void StoreDataIntoFile(hid_t         h5fileId,
                       const float  *data,
                       const size_t  edgeSize,
                       const size_t  snapshotId,
                       const size_t  iteration)
{
    hid_t   dataset_id, dataspace_id, group_id, attribute_id;
    hsize_t dims[2] = {edgeSize, edgeSize};

    string groupName = "Timestep_" + to_string((unsigned long long) snapshotId);

    // Create a group named "/Timestep_snapshotId" in the file.
    group_id = H5Gcreate(h5fileId,
                         groupName.c_str(),
                         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);


    // Create the data space. (2D matrix)
    dataspace_id = H5Screate_simple(2, dims, NULL);

    // create a dataset for temperature and write data
    string datasetName = "Temperature";
    dataset_id = H5Dcreate(group_id,
                           datasetName.c_str(),
                           H5T_NATIVE_FLOAT,
                           dataspace_id,
                           H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(dataset_id,
             H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
             data);

    // close dataset
    H5Sclose(dataspace_id);


    // write attribute
    string atributeName="Time";
    dataspace_id = H5Screate(H5S_SCALAR);
    attribute_id = H5Acreate2(group_id, atributeName.c_str(),
                              H5T_IEEE_F64LE, dataspace_id,
                              H5P_DEFAULT, H5P_DEFAULT);

    double snapshotTime = double(iteration);
    H5Awrite(attribute_id, H5T_IEEE_F64LE, &snapshotTime);
    H5Aclose(attribute_id);


    // Close the dataspace.
    H5Sclose(dataspace_id);

    // Close to the dataset.
    H5Dclose(dataset_id);

    // Close the group.
    H5Gclose(group_id);
} // end of StoreDataIntoFile
//------------------------------------------------------------------------------

/**
 * Store time step into output file using parallel version of HDF5
 * @param [in] h5fileId   - handle to the output file
 * @param [in] data       - data to write
 * @param [in] edgeSize   - size of the domain
 * @param [in] tileWidth  - width of the tile
 * @param [in] tileHeight - height of the tile
 * @param [in] tilePosX   - position of the tile in the grid (X-dir)
 * @param [in] tilePosY   - position of the tile in the grid (Y-dir)
 * @param [in] snapshotId - snapshot id
 * @param [in] iteration  - id of iteration
 */
void StoreDataIntoFileParallel(hid_t h5fileId,
                               const float *data,
                               const size_t edgeSize,
                               const size_t tileWidth, const size_t tileHeight,
                               const size_t tilePosX, const size_t tilePosY,
                               const size_t snapshotId,
                               const size_t iteration)
{
    hid_t dataset_id, dataspace_id, group_id, attribute_id, memspace_id;
    const hsize_t dims[2] = { edgeSize, edgeSize };
    const hsize_t offset[2] = { tilePosY, tilePosX };
    const hsize_t tile_dims[2] = { tileHeight, tileWidth };
    const hsize_t core_dims[2] = { tileHeight - 4, tileWidth - 4 };
    const hsize_t core_offset[2] = { 2, 2 };

    string groupName = "Timestep_" + to_string((unsigned long)snapshotId);

    // Create a group named "/Timestep_snapshotId" in the file.
    group_id = H5Gcreate(h5fileId,
                         groupName.c_str(),
                         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Create the data space in the output file. (2D matrix)
    dataspace_id = H5Screate_simple(2, dims, NULL);

    // create a dataset for temperature and write data
    string datasetName = "Temperature";
    dataset_id = H5Dcreate(group_id,
                           datasetName.c_str(),
                           H5T_NATIVE_FLOAT,
                           dataspace_id,
                           H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // create the data space in memory representing local tile. (2D matrix)
    memspace_id = H5Screate_simple(2, tile_dims, NULL);

    // select appropriate block of the local tile. (without halo zones)
    H5Sselect_hyperslab(memspace_id, H5S_SELECT_SET, core_offset, NULL, core_dims, NULL);

    // select appropriate block of the output file, where local tile will be placed.
    H5Sselect_hyperslab(dataspace_id, H5S_SELECT_SET, offset, NULL, core_dims, NULL);

    // setup collective write using MPI parallel I/O
    hid_t hPropList = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(hPropList, H5FD_MPIO_COLLECTIVE);

    H5Dwrite(dataset_id, H5T_NATIVE_FLOAT, memspace_id, dataspace_id, hPropList, data);

    // close memory spaces and property list
    H5Sclose(memspace_id);
    H5Sclose(dataspace_id);
    H5Pclose(hPropList);

    // write attribute
    string attributeName = "Time";
    dataspace_id = H5Screate(H5S_SCALAR);
    attribute_id = H5Acreate2(group_id, attributeName.c_str(),
                              H5T_IEEE_F64LE, dataspace_id,
                              H5P_DEFAULT, H5P_DEFAULT);

    double snapshotTime = double(iteration);
    H5Awrite(attribute_id, H5T_IEEE_F64LE, &snapshotTime);
    H5Aclose(attribute_id);

    // close the dataspace
    H5Sclose(dataspace_id);

    // close the dataset and the group
    H5Dclose(dataset_id);
    H5Gclose(group_id);
}
//------------------------------------------------------------------------------

/**
 * Main function of the project
 * @param [in] argc
 * @param [in] argv
 * @return
 */
int main(int argc, char *argv[])
{
    int rank, size;

    ParseCommandline(argc, argv, parameters);

    // Initialize MPI
    MPI_Init(&argc, &argv);

    // Get MPI rank and size
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);


    if (rank == 0)
    {
        // Create material properties and load from file
        materialProperties.LoadMaterialData(parameters.materialFileName, true);
        parameters.edgeSize = materialProperties.edgeSize;

        parameters.PrintParameters();
    }
    else
    {
        // Create material properties and load from file
        materialProperties.LoadMaterialData(parameters.materialFileName, false);
        parameters.edgeSize = materialProperties.edgeSize;
    }

    if (parameters.edgeSize % size)
    {
        if (rank == 0)
            printf("ERROR: number of MPI processes is not a divisor of N\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (parameters.IsRunSequntial())
    {
        if (rank == 0)
        {
            // Memory allocation for output matrices.
            seqResult = (float*)_mm_malloc(materialProperties.nGridPoints * sizeof(float), DATA_ALIGNMENT);

            SequentialHeatDistribution(seqResult,
                                       materialProperties,
                                       parameters,
                                       parameters.outputFileName);
        }
    }

    if (parameters.IsRunParallel())
    {
        // Memory allocation for output matrix.
        if (rank == 0)
            parResult = (float*) _mm_malloc(materialProperties.nGridPoints * sizeof(float), DATA_ALIGNMENT);
        else
            parResult = NULL;

        ParallelHeatDistribution(parResult,
                                 materialProperties,
                                 parameters,
                                 parameters.outputFileName);
    }

    // Validate the outputs
    if (parameters.IsValidation() && rank == 0)
    {
        if (parameters.debugFlag)
        {
            printf("---------------- Sequential results ---------------\n");
            PrintArray(seqResult, materialProperties.edgeSize);

            printf("----------------- Parallel results ----------------\n");
            PrintArray(parResult, materialProperties.edgeSize);
        }

        if (VerifyResults(seqResult, parResult, parameters, 0.001f))
            printf("Verification OK\n");
        else
            printf("Verification FAILED\n");
    }

    /* Memory deallocation*/
    _mm_free(seqResult);
    _mm_free(parResult);

    MPI_Finalize();

    return EXIT_SUCCESS;
} // end of main
//------------------------------------------------------------------------------