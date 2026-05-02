#pragma once

#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>

using std::ifstream;
using std::fstream;
using std::hex;
using std::string;
using std::cout;
using std::endl;

struct ImageData
{
    string hexString;

    /// <summary>Gets the raw hex bytes of the image</summary>
    /// <param name="imageLocation">: Provide the full path to the image</param>
    void convertImageToHex(const string& imageLocation);

    /// <summary>Used to retrieve chunks of hex data from the hex string</summary>
    /// <param name="length">: The amount of hex pairs to return I.E. 1 -> FF</param>
    /// <param name="returnHex">: Provide the string that will store the hex bytes</param>
    void getHexChunk(int length, string& returnHex);

    /// <summary>Returns the decimal value of the passed hex string</summary>
    /// <param name="hexChunk">: The hex chunk you want the value of</param>
    static int getHexValue(const string& hexChunk);

private:
    int currentPos = 0;
};