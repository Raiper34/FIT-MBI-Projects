/*
 * KKO Project 2017 - GIF to BMP Converter
 * Author: Filip Gulan (xgulan00)
 * Mail: xgulan00@stud.fit.vutbr.cz
 * Date: 7.5.2017
 * Name: gif2bmp.cpp
 * Description: Gif2Bmp library source file
 */

#include <iostream>
#include <bitset>
#include <tgmath.h>
#include <vector>
#include <iomanip>
#include "gif2bmp.h"

#define SUCCESS 0
#define FAILURE 1
#define DEBUG false //only for debuging purpose
#define HEXA_SYSTEM 16
#define BINARY_SYSTEM 2
#define  BMP_HEADER_SIZE 14
#define DIB_HEADER_SIZE 40
#define BYTE 8
#define ENDLESS 1
#define MAX_LZW_SIZE 12


using namespace std;


/**
 * Print debug informations if debug is ON
 * @param text debug text to print
 */
void printDebug(string text)
{
    if(DEBUG)
    {
        cout << text << endl;
    }
}

/**
 * Convert string of hexa numbers into normal string
 * @param data to convert
 * @return converted data
 */
string hexaString(string data)
{
    string hexaString = "";
    for(int i = 0; i < data.length(); i += 2) //we go trought pairs
    {
        string pair = data.substr(i,2);
        char character = (char)(int)strtol(pair.c_str(), NULL, HEXA_SYSTEM); //convert pair into character
        hexaString.push_back(character);
    }
    return hexaString;
}

/**
 * Convert little endian string to big endian or oppositely
 * @param littleEndian little endian string
 * @return big endian string
 */
string littleToBigEndian(string littleEndian)
{
    string bigEndian = "";
    for(int i = 0; i < littleEndian.length()/2; i++) //we go trought pairs
    {
        bigEndian.insert(0, littleEndian.substr(i * 2, 2));
    }
    return bigEndian;
}

/**
 * Convert int value to hexa string
 * @param number to convert
 * @param byteCount number of bytes
 * @return converted number
 */
string intToHex(int number, int byteCount) //1 byte = 2 hexa characters
{
    stringstream stream;
    stream << hex << number; //conversion to hex
    string hexa = stream.str();
    if(hexa.length() % 2 != 0) //for even count
    {
        hexa.insert(0, "0");
    }
    hexa = littleToBigEndian(hexa);
    for(int i = hexa.length()/2; i < byteCount; i++) //we add 00 for required number of bytes
    {
        hexa.append("00");
    }
    return hexa;
}

/**
 * Make align to bmp It need to be /4
 * @param colorBytes number of used bytes per color
 * @param width of row
 * @return align number
 */
int makebmpAlign(int colorBytes, int width)
{
    int align = 0;
    while(((colorBytes*width) + align) % 4 != 0) //because we want to /4
    {
        align++;
    }
    return align;
}

/**
 * Get size of image
 * @param image which size we want
 * @return size of image
 */
int getSizeOfImage(string image)
{
    unsigned int i = 0;
    return image.length()/2; //because 2 characters represent one byte
}

/**
 * Convert interlaced image to noninterlaced to be show in bmp
 * @param interlaced image data
 * @param width of interlaced image
 * @param height of interlaced image
 * @return non interlaced image data
 */
vector<string> interlacedToNonInterlaced(vector<string> interlaced, int width, int height)
{
    vector<string> nonInterlaced;
    int count8start0 = (int)ceil((double)height/8);
    int count8start4 = (int)ceil((double)(height - 4)/8);
    int count4start2 = (int)ceil((double)(height - 2)/4);
    int count2start1 = (int)ceil((double)(height - 1)/2);
    for(int i = 0; i < count8start0; i++) //iterate trought block of passes. 1 4 3 4 2 4 3 4 and then repeat
    {
        for(int x = 0; x < width; x++) //1st pass
        {
            nonInterlaced.push_back(interlaced[x + i * width]);
        }
        for(int x = 0; x < width; x++) //4th pass
        {
            int y = count8start0 + count8start4 + count4start2 + (i * 4);
            if(x + y * width < width * height)
            {
                nonInterlaced.push_back(interlaced[x + y * width]);
            }
        }
        for(int x = 0; x < width; x++) //3rd pass
        {
            int y = count8start0 + count8start4 + (i * 2);
            if(x + y * width < width * height)
            {
                nonInterlaced.push_back(interlaced[x + y * width]);
            }
        }
        for(int x = 0; x < width; x++) //4th pass
        {
            int y = count8start0 + count8start4 + count4start2 + (i * 4) + 1;
            if(x + y * width < width * height)
            {
                nonInterlaced.push_back(interlaced[x + y * width]);
            }
        }
        for(int x = 0; x < width; x++) //2nd pass
        {
            int y = count8start0 + i;
            if(x + y * width < width * height)
            {
                nonInterlaced.push_back(interlaced[x + y * width]);
            }
        }
        for(int x = 0; x < width; x++) //4th pass
        {
            int y = count8start0 + count8start4 + count4start2 + (i * 4) + 2;
            if(x + y * width < width * height)
            {
                nonInterlaced.push_back(interlaced[x + y * width]);
            }
        }
        for(int x = 0; x < width; x++) //3rd pass
        {
            int y = count8start0 + count8start4 + (i * 2) + 1;
            if(x + y * width < width * height)
            {
                nonInterlaced.push_back(interlaced[x + y * width]);
            }
        }
        for(int x = 0; x < width; x++) //4th pass
        {
            int y = count8start0 + count8start4 + count4start2 + (i * 4) + 3;
            if(x + y * width < width * height)
            {
                nonInterlaced.push_back(interlaced[x + y * width]);
            }
        }
    }

    return nonInterlaced;
}

/**
 * Make bmp string
 * @param width of creating image
 * @param height of creating image
 * @param pixels of image
 * @return bmp string image
 */
string makeBMP(int width, int height, vector<string> pixels)
{
    /******* Declarations ********/
    string bmp = "";
    int bytesPerColor = 3;
    int alignSize = makebmpAlign(bytesPerColor, width);
    int bmpSize = BMP_HEADER_SIZE + DIB_HEADER_SIZE + (width * height * bytesPerColor) + (height * alignSize);

    /********** Coding **********/
    //BMP header - 14 bytes
    bmp.append("424d"); //bfType BM
    bmp.append(intToHex(bmpSize, 4)); //bfSize size of bmp file 54 + pixel array
    bmp.append("0000"); //bfReserved1 unused
    bmp.append("0000"); //bfReserved2 unused
    bmp.append(intToHex(BMP_HEADER_SIZE + DIB_HEADER_SIZE, 4)); //bfOffBits offset where image data starts !IMPORTANT!

    //DIB header - 40 bytes
    bmp.append(intToHex(DIB_HEADER_SIZE, 4)); //biSize  size of DIB header !IMPORTANT!
    bmp.append(intToHex(width, 4)); //biWidth width of image !IMPORTANT!
    bmp.append(intToHex(height, 4)); //biHeight height of image !IMPORTANT!
    bmp.append("0100"); //biPlanes bit plates
    bmp.append(intToHex(bytesPerColor * BYTE, 2)); //biBitCount bits per pixel
    bmp.append("00000000"); //biCompresion type of compression
    bmp.append("00000000"); //biSizeImage size of image in bits, if image is uncompressed can be 0
    bmp.append("00000000"); //biXpelsPerMeter
    bmp.append("00000000"); //biYpelsPerMeter
    bmp.append("00000000"); //biCirUsed number of colors that are used 0 all used
    bmp.append("00000000"); //biCirimportant number of colors that are important 0 all important

    //Align image data need to be /4
    string align = "";
    for(int i = 0; i < alignSize; i++)
    {
        align.append("00");
    }

    //Pixel array - 16 bytes
    for(int y = height - 1; y >= 0; y--) //because data in gif are from top left to down right, but in bmp from bottom left to top right
    {
        for(int x = 0; x < width; x++)
        {
            bmp.append(pixels[x + y * width]); //BGR
        }
        bmp.append(align);
    }
    return bmp;
}

/**
 * Remove comment extension from gif file, there is nothing important for us
 * @param gif image
 * @return gif image without comment extension
 */
string removeCommentExtension(string gif) //FE without 21 but with label
{
    printDebug("Comment extension");
    gif.erase(0, 2); //comment label
    while(gif.substr(0, 2).compare("00") != 0)
    {
        int subBlockSize = strtol(gif.substr(0, 2).c_str(), NULL, 16);
        gif.erase(0, 2); //blocksize
        for(int i = 0; i < subBlockSize; i++) //block data
        {
            gif.erase(0, 2);
        }
    }
    gif.erase(0, 2); //block terminator
    return gif;
}

/**
 * Remove plain text extension or application extension, there is nothing important for us
 * @param gif image
 * @return gif image without theese extensions
 */
string removePlainTextOrApplicationExtension(string gif) //01/FF without 21 but with label
{
    printDebug("PlainText or Application extension");
    gif.erase(0, 2); //plain text label
    int blockSize = strtol(gif.substr(0, 2).c_str(), NULL, HEXA_SYSTEM);
    gif.erase(0, 2);
    for(int i = 0; i < blockSize; i++) //block data like text grid positions, width...
    {
        gif.erase(0, 2);
    }
    while(gif.substr(0, 2).compare("00") != 0)
    {
        int subBlockSize = strtol(gif.substr(0, 2).c_str(), NULL, HEXA_SYSTEM);
        gif.erase(0, 2); //blocksize
        for(int i = 0; i < subBlockSize; i++) //block data
        {
            gif.erase(0, 2);
        }
    }
    gif.erase(0, 2); //block terminator
    return gif;
}

/**
 * Decode LZW data string to vector image pixels
 * @param lzwBlock to decode
 * @param lzwMinimumCodeSize
 * @param globalColorTable
 * @return vector of image pixels
 */
vector<string> decodeLZW(string lzwBlock, int lzwMinimumCodeSize, vector<string> globalColorTable)
{
    /******** Declarations *********/
    vector<string> image;
    vector<string> lzwTable;
    int index = 0;
    int lzwCodeSize = 0;
    string indexValue = "";
    string oldIndexValue = "";

    /******* Algorithm ***********/
    //Initialization
    lzwTable.clear();
    lzwTable = globalColorTable;
    lzwTable.push_back("CLEARCODE");
    lzwTable.push_back("STOPCODE");
    int clearCode = lzwTable.size() - 2;
    int stopCode = lzwTable.size() - 1;
    lzwCodeSize = lzwMinimumCodeSize + 1;
    //First iteration outside loop
    index = strtol(lzwBlock.substr(lzwBlock.length() - lzwCodeSize, lzwCodeSize).c_str(), NULL, 2);
    if(index == clearCode)
    {
        printDebug("CLEAR CODE");
        lzwBlock.erase(lzwBlock.length() - lzwCodeSize, lzwCodeSize);
    }
    index = strtol(lzwBlock.substr(lzwBlock.length() - lzwCodeSize, lzwCodeSize).c_str(), NULL, 2); //first non Clear code
    lzwBlock.erase(lzwBlock.length() - lzwCodeSize, lzwCodeSize);
    //lzwTable.push_back(oldIndexValue + indexValue.substr(0, 6));
    indexValue = lzwTable[index];
    string tempValue = indexValue;
    while(tempValue.length() > 0)
    {
        image.push_back(tempValue.substr(0, 6)); //6 because one color consist of 6 hexa characters and it is 3 bytes
        tempValue.erase(0, 6);
    }
    oldIndexValue = indexValue;
    //Loop
    while(ENDLESS)
    {
        index = strtol(lzwBlock.substr(lzwBlock.length() - lzwCodeSize, lzwCodeSize).c_str(), NULL, 2);
        lzwBlock.erase(lzwBlock.length() - lzwCodeSize, lzwCodeSize);
        if(index == stopCode) //we found stop code
        {
            printDebug("STOP CODE");
            break;
        }
        if(index == clearCode) //we found clear code
        {
            printDebug("CLEAR CODE");
            lzwTable.clear();
            lzwTable = globalColorTable;
            lzwTable.push_back("CLEARCODE");
            lzwTable.push_back("STOPCODE");
            int clearCode = lzwTable.size() - 2;
            int stopCode = lzwTable.size() - 1;
            lzwCodeSize = lzwMinimumCodeSize + 1;
            index = strtol(lzwBlock.substr(lzwBlock.length() - lzwCodeSize, lzwCodeSize).c_str(), NULL, 2); //first non Clear code
            lzwBlock.erase(lzwBlock.length() - lzwCodeSize, lzwCodeSize);
            //lzwTable.push_back(oldIndexValue + indexValue.substr(0, 6));
            indexValue = lzwTable[index];
            string tempValue = indexValue;
            while(tempValue.length() > 0)
            {
                image.push_back(tempValue.substr(0, 6));
                tempValue.erase(0, 6);
            }
            oldIndexValue = indexValue;
            continue;
        }
        //If code stream is in table
        if(lzwTable.size() > index) //index is in table
        {
            indexValue = lzwTable[index];
            lzwTable.push_back(oldIndexValue + indexValue.substr(0, 6));
        }
        else //index is not in table
        {
            lzwTable.push_back(oldIndexValue + oldIndexValue.substr(0, 6));
            indexValue = oldIndexValue + oldIndexValue.substr(0, 6);
        }
        //outputs
        tempValue = indexValue;
        while(tempValue.length() > 0)
        {
            image.push_back(tempValue.substr(0, 6));
            tempValue.erase(0, 6);
        }
        oldIndexValue = indexValue;
        //Larger data
        if(pow(2, lzwCodeSize) - 1 <= lzwTable.size() - 1 && lzwCodeSize < MAX_LZW_SIZE)
        {
            printDebug("LZWcode++");
            lzwCodeSize++;
        }
    }
    return image;
}

/**
 * Remove gif header from gif and check if it is gif file
 * @param gif file
 * @return SUCCESS if it gif file, otherwise FAILURE
 */
int gifHeader(string *gif)
{
    if(gif->substr(0, 6).compare("474946") == 0) //gif signature GIF
    {
        printDebug("GIF signature");
        gif->erase(0, 6);
    }
    else
    {
        cerr << "Input need to be GIF file!" << endl;
        return FAILURE;
    }

    if(gif->substr(0, 6).compare("383961") == 0) //gif version 89a
    {
        printDebug("GIF version 89a");
        gif->erase(0, 6);
    }
    else if(gif->substr(0, 6).compare("383761") == 0) //gif version 87a
    {
        printDebug("GIF version 87a");
        gif->erase(0, 6);
    }
    else
    {
        cerr << "Input need to be GIF file!" << endl;
        return FAILURE;
    }

    return SUCCESS;
}

/**
 * Remove logical screen descriptor from gif file and return required informations
 * @param gif file
 * @param globalColorTableFlag to return
 * @param sizeOfGlobalColorTable to return
 * @return modified gif
 */
string logicalScreenDescriptor(string gif, bool *globalColorTableFlag, unsigned int *sizeOfGlobalColorTable)
{
    unsigned int widthOfLogicScreen = strtol(littleToBigEndian(gif.substr(0, 4)).c_str(), NULL, HEXA_SYSTEM); //width of logical screen
    printDebug("Width Of LogicalScreen: " + to_string(widthOfLogicScreen));
    gif.erase(0, 4);

    unsigned int heightOfLogicScreen = strtol(littleToBigEndian(gif.substr(0, 4)).c_str(), NULL, HEXA_SYSTEM); //height of logical screen
    printDebug("Height Of LogicalScreen: " + to_string(heightOfLogicScreen));
    gif.erase(0, 4);

    string globalColorTableFlagsArray = bitset<8>(strtol(gif.substr(0, 2).c_str(), NULL, HEXA_SYSTEM)).to_string(); //Global Color table flags
    printDebug("Global Color Table Flags Array: " + globalColorTableFlagsArray);
    gif.erase(0, 2);
    *globalColorTableFlag = strtol(globalColorTableFlagsArray.substr(0, 1).c_str(), NULL, BINARY_SYSTEM); //global color table flag
    globalColorTableFlagsArray.erase(0, 1);
    int colorResolution = strtol(globalColorTableFlagsArray.substr(0, 3).c_str(), NULL, BINARY_SYSTEM) - 1; //color resolution
    globalColorTableFlagsArray.erase(0, 3);
    bool sortFlag= strtol(globalColorTableFlagsArray.substr(0, 1).c_str(), NULL, BINARY_SYSTEM); //sort flag
    globalColorTableFlagsArray.erase(0, 1);
    *sizeOfGlobalColorTable = pow(2, strtol(globalColorTableFlagsArray.substr(0, 3).c_str(), NULL, BINARY_SYSTEM) + 1); //size of global color table
    globalColorTableFlagsArray.erase(0, 3);
    printDebug("Global Color Table Flag: " + to_string(*globalColorTableFlag) + " Color Resolution: " + to_string(colorResolution) + " Sort flag: " + to_string(sortFlag) + " Size of Global Color Table: " + to_string(*sizeOfGlobalColorTable));

    int backgroundColor = strtol(gif.substr(0, 2).c_str(), NULL, HEXA_SYSTEM); //Background color
    gif.erase(0, 2);
    printDebug("Background color: " + to_string(backgroundColor));

    int aspectRation = (strtol(gif.substr(0, 2).c_str(), NULL, HEXA_SYSTEM) == 0)? 0 : (strtol(gif.substr(0, 2).c_str(), NULL, HEXA_SYSTEM) + 15)/64; //aspect ratio
    gif.erase(0, 2);
    printDebug("Aspect Ratio: " + to_string(aspectRation));

    return gif;
}

/**
 * Get color table and return size of it
 * @param gif file
 * @param sizeOfGlobalColorTable to return
 * @return global color table
 */
vector<string> colorTable(string *gif, unsigned int sizeOfGlobalColorTable)
{
    vector<string> globalColorTable;
    printDebug("Global Color table allowed");
    for(int i = 0; i < sizeOfGlobalColorTable; i++)
    {
        string red = gif->substr(0, 2);
        gif->erase(0, 2);
        string green = gif->substr(0, 2);
        gif->erase(0, 2);
        string blue = gif->substr(0, 2);
        gif->erase(0, 2);
        globalColorTable.push_back(blue + green + red);
        //cout << to_string(i) + ": " + red + green + blue << endl;
    }
    return globalColorTable;
}

/**
 * Remove graphic control extension
 * @param gif file
 * @return modified gif
 */
string graphicControlExtension(string gif)
{
    printDebug("Graphic control extension");
    gif.erase(0, 2);

    int blockSize = strtol(gif.substr(0, 2).c_str(), NULL, HEXA_SYSTEM); //block size
    printDebug("Block size: " + to_string(blockSize));
    gif.erase(0, 2);

    string blockFlagsArray = bitset<8>(strtol(gif.substr(0, 2).c_str(), NULL, HEXA_SYSTEM)).to_string(); //Global Color table flags
    printDebug("Block Flags Array: " + blockFlagsArray);
    gif.erase(0, 2);
    blockFlagsArray.erase(0, 3); //reserved
    int disposalMethod = strtol(blockFlagsArray.substr(0, 3).c_str(), NULL, BINARY_SYSTEM); //disposal method
    blockFlagsArray.erase(0, 3);
    blockFlagsArray.erase(0, 1); //user input flag
    bool transparentColorFlag = strtol(blockFlagsArray.substr(0, 1).c_str(), NULL, BINARY_SYSTEM); //transparent color flag
    blockFlagsArray.erase(0, 1);
    printDebug("Disposal method: " + to_string(disposalMethod) + " Transparent color flag: " + to_string(transparentColorFlag));

    gif.erase(0, 4); //Delay time

    int transparentcyIndex = strtol(gif.substr(0, 2).c_str(), NULL, HEXA_SYSTEM); //Transparent color index
    printDebug("Transparency index: " + to_string(transparentcyIndex));
    gif.erase(0, 2);

    if(gif.substr(0, 2).compare("00") == 0) //Block terminator
    {
        printDebug("Block Terminator");
        gif.erase(0, 2);
    }

    return gif;
}

/**
 * Remove image decriptor and get required data
 * @param gif file
 * @param localColorTableFlag to return
 * @param interlacedFlag to return
 * @param sizeOfLocalColorTable to return
 * @param imageWidth to return
 * @param imageHeight to return
 * @return modified gif
 */
string imageDescriptor(string gif, bool *localColorTableFlag, bool *interlacedFlag, unsigned int *sizeOfLocalColorTable, unsigned int *imageWidth, unsigned int *imageHeight)
{
    if(gif.substr(0, 2).compare("2c") == 0) //Image separator
    {
        printDebug("Image descriptor");
        gif.erase(0, 2);
    }

    unsigned int imageLeftPosition = strtol(littleToBigEndian(gif.substr(0, 4)).c_str(), NULL, HEXA_SYSTEM); //Image left position
    printDebug("Image left position: " + to_string(imageLeftPosition));
    gif.erase(0, 4);

    unsigned int imageTopPosition = strtol(littleToBigEndian(gif.substr(0, 4)).c_str(), NULL, HEXA_SYSTEM); //Image top position
    printDebug("Image top position: " + to_string(imageTopPosition));
    gif.erase(0, 4);

    *imageWidth = strtol(littleToBigEndian(gif.substr(0, 4)).c_str(), NULL, HEXA_SYSTEM); //Image width
    printDebug("Image width: " + to_string(*imageWidth));
    gif.erase(0, 4);

    *imageHeight = strtol(littleToBigEndian(gif.substr(0, 4)).c_str(), NULL, HEXA_SYSTEM); //Image height
    printDebug("Image height: " + to_string(*imageHeight));
    gif.erase(0, 4);

    string imageDescriptorFlagsArray = bitset<8>(strtol(gif.substr(0, 2).c_str(), NULL, HEXA_SYSTEM)).to_string(); //Image descriptro flags
    printDebug("Image descriptor Flags Array: " + imageDescriptorFlagsArray);
    gif.erase(0, 2);
    *localColorTableFlag = strtol(imageDescriptorFlagsArray.substr(0, 1).c_str(), NULL, BINARY_SYSTEM); //Local color table flag
    imageDescriptorFlagsArray.erase(0, 1);
    *interlacedFlag = strtol(imageDescriptorFlagsArray.substr(0, 1).c_str(), NULL, BINARY_SYSTEM); //Interlaced flag
    imageDescriptorFlagsArray.erase(0, 1);
    int localSortFlag = strtol(imageDescriptorFlagsArray.substr(0, 1).c_str(), NULL, BINARY_SYSTEM); //Sort flag
    imageDescriptorFlagsArray.erase(0, 1);
    imageDescriptorFlagsArray.erase(0, 2); //Reserved
    *sizeOfLocalColorTable = pow(2, strtol(imageDescriptorFlagsArray.substr(0, 3).c_str(), NULL, BINARY_SYSTEM) + 1); //size of local color table
    imageDescriptorFlagsArray.erase(0, 3);
    printDebug("Local color table flag: " + to_string(*localColorTableFlag) + " Interlaced Flag: " + to_string(*interlacedFlag) + " Sort flag: " + to_string(localSortFlag) + " Size of local table: " + to_string(*sizeOfLocalColorTable));

    return gif;
}

/**
 * Get table data/lzw block
 * @param gif file
 * @param lzwMinimumCodeSize to return
 * @return table data/lzw block
 */
string tableData(string *gif, int *lzwMinimumCodeSize, unsigned int *gifSize)
{
    *gifSize = 0;
    *lzwMinimumCodeSize = strtol(gif->substr(0, 2).c_str(), NULL, 16); //lzw minimum code size
    gif->erase(0, 2);
    printDebug("LZW minimum code size: " + to_string(*lzwMinimumCodeSize));

    string lzwBlock = "";
    while(ENDLESS)
    {
        int sizeOfLzwBlock = strtol(gif->substr(0, 2).c_str(), NULL, HEXA_SYSTEM); //size of lzw block
        gif->erase(0, 2);
        (*gifSize)++;
        for(int i = 0; i < sizeOfLzwBlock; i++) //lzw block
        {
            lzwBlock.insert(0, bitset<BYTE>(strtol(gif->substr(0, 2).c_str(), NULL, HEXA_SYSTEM)).to_string());
            gif->erase(0, 2);
            (*gifSize)++;
        }
        if(gif->substr(0, 2).compare("00") == 0) //lzw block terminator
        {
            gif->erase(0, 2);
            (*gifSize)++;
            break;
        }
    }
    return lzwBlock;
}

/**
 * Decode gif image and return image data in *image
 * @param gif data
 * @param imageWidthReturn width of image to return
 * @param imageHeightReturn height of image to return
 * @param image data to return
 * @param gifSize to return
 * @return SUCCESS or FAILURE on error
 */
int decodeGIF(string gif, int *imageWidthReturn, int *imageHeightReturn, vector<string> *image, unsigned int *gifSize)
{
    /****** Declarations ******/
    bool globalColorTableFlag;
    unsigned int sizeOfGlobalColorTable;
    vector<string> globalColorTable; //color table
    bool localColorTableFlag;
    bool interlacedFlag;
    unsigned int sizeOfLocalColorTable;
    unsigned int imageWidth;
    unsigned int imageHeight;
    int lzwMinimumCodeSize;
    string lzwBlock;
    bool imageBlockProceseed = false;
    unsigned int lzwSize = 0;

    /******* Decoding ********/
    //Gif Header
    if(gifHeader(&gif) != SUCCESS)
    {
        return FAILURE;
    }
    //Logical Screen descriptor
    gif = logicalScreenDescriptor(gif, &globalColorTableFlag, &sizeOfGlobalColorTable);
    //Global color table
    if(globalColorTableFlag)
    {
        globalColorTable = colorTable(&gif, sizeOfGlobalColorTable);
    }

    while(gif.substr(0, 2).compare("3b") != 0) //while not trailer
    {
        if(gif.substr(0, 2).compare("21") == 0) //Extensions
        {
            printDebug("Extension Introducter");
            gif.erase(0, 2);

            if(gif.substr(0, 2).compare("01") == 0 || gif.substr(0, 2).compare("ff") == 0) //application extension or plain text extension
            {
                gif = removePlainTextOrApplicationExtension(gif);
            }
            else if(gif.substr(0, 2).compare("fe") == 0) //comment extension
            {
                gif = removeCommentExtension(gif);
            }
            else //Graphic control extension f9
            {
                gif = graphicControlExtension(gif);
                if (gif.substr(0, 2).compare("21") == 0) //plain text
                {
                    printDebug("Extension Introducter");
                    gif.erase(0, 2);
                    gif = removePlainTextOrApplicationExtension(gif);
                }
                else //Image data
                {
                    if(imageBlockProceseed) //test if it is static gif
                    {
                        break;
                    }
                    //Image descriptor
                    gif = imageDescriptor(gif, &localColorTableFlag, &interlacedFlag, &sizeOfLocalColorTable, &imageWidth, &imageHeight);
                    //Local color table
                    if (localColorTableFlag)
                    {
                        globalColorTable = colorTable(&gif, sizeOfLocalColorTable);
                    }
                    //Table data
                    lzwBlock = tableData(&gif, &lzwMinimumCodeSize, &lzwSize);
                    imageBlockProceseed = true;

                }
            }
        }
        else //Image data
        {
            if(imageBlockProceseed) //test if it is static gif
            {
                break;
            }
            //Image descriptor
            gif = imageDescriptor(gif, &localColorTableFlag, &interlacedFlag, &sizeOfLocalColorTable, &imageWidth, &imageHeight);
            //Local color table
            if (localColorTableFlag)
            {
                globalColorTable = colorTable(&gif, sizeOfLocalColorTable);
            }
            //Table data
            lzwBlock = tableData(&gif, &lzwMinimumCodeSize, &lzwSize);
            imageBlockProceseed = true;
        }
    }
    //Trailer
    printDebug("Trailer");
    gif.erase(0, 2);

    //LZW decoding
    *image = decodeLZW(lzwBlock, lzwMinimumCodeSize, globalColorTable);
    if(interlacedFlag) //interlaced flag is set
    {
        printDebug("INTERLACING");
        *image = interlacedToNonInterlaced(*image, imageWidth, imageHeight);
    }

    //Return
    *imageWidthReturn = imageWidth;
    *imageHeightReturn = imageHeight;
    *gifSize = lzwSize;
    return SUCCESS;
}

/**
 * Main library function that run decoding of gif and coding to bmp
 * @param gif2bmp structure
 * @param inputFile ifstream pointer to opened input file
 * @param outputFile  ofstream pointer to opened output file
 * @return SUCCESS or FAILLURE on error
 */
int gif2bmp(tGIF2BMP *gif2bmp, ifstream *inputFile, ofstream *outputFile)
{
    //Input
    string inputString;
    string line;
    unsigned char x;
    *inputFile >> noskipws;
    stringstream buffer;
    if(inputFile->is_open()) //if file is open we read from it
    {
        while(*inputFile >> x)
        {
            buffer << hex << setw(2) << setfill('0') << (int)x;
        }
    }
    else //we read from strin
    {
        while(cin >> x)
        {
            buffer << hex << setw(2) << setfill('0') << (int)x;
        }
    }
    inputString = buffer.str();

    //GIF
    unsigned gifSize;
    int imageHeight = 0;
    int imageWidth = 0;
    vector<string> image;
    if(decodeGIF(inputString, &imageWidth, &imageHeight, &image, &gifSize) != SUCCESS)
    {
        return FAILURE;
    }

    //Debug
    printDebug("REAL: " + to_string(image.size() * 6));
    printDebug("EXCEPTING: " + to_string(imageWidth * imageHeight * 6));

    //BMP
    string bmp = makeBMP(imageWidth, imageHeight, image);
    unsigned int bmpSize = getSizeOfImage(bmp) - 54; // size of bmp - headers

    //Output
    if(outputFile->is_open()) //if file is open we write from it
    {
        *outputFile << hexaString(bmp)  << endl;
    }
    else //we write to stdout
    {
        cout << hexaString(bmp) << endl;
    }

    //Info for log
    gif2bmp->bmpSize = bmpSize;
    gif2bmp->gifSize = gifSize;

    return SUCCESS;
}
