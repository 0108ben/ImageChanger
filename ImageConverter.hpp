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
    static void convertImageToHex(const string& imageLocation, string& hexString);

    void getHexChunk(const string& hexString, int length, string& returnHex);

    static int getHexValue(const string& hexChunk);

private:
    int currentPos = 0;
};