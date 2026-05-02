#pragma once

#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <map>
#include <unordered_map>

using std::ifstream;
using std::fstream;
using std::hex;
using std::string;
using std::cout;
using std::endl;

inline const string pngSignature = "89 50 4e 47 0d 0a 1a 0a";

static const string hexIHDR = "49 48 44 52";
static const string hexIDAT = "49 44 41 54";
static const string hexIEND = "49 45 4e 44";

struct ImageData
{

    static const std::unordered_map<int, string> colourType;

    string hexString;

    /// <summary>Gets the raw hex bytes of the image</summary>
    /// <param name="imageLocation">: Provide the full path to the image</param>
    void convertImageToHex(const string& imageLocation);

    /// <summary>Used to retrieve all data from a hex chunk: size, type, data, CRC</summary>
    /// <param name="chunk">: Stores all data from the chunk</param>
    void getHexChunk(std::map<string, string>& chunk);

    /// <summary>Used to retrieve small chunks of hex data from the hex string</summary>
    /// <param name="length">: The amount of hex pairs to return I.E. 1 -> FF</param>
    /// <param name="returnHex">: Provide the string that will store the hex bytes</param>
    /// <param name="startingPoint">: Starting position of the chunk, automatically worked out if not specified</param>
    void getSmallHexChunk(string& returnHex, int length, int startingPoint = -1);

    /// <summary>Used to retrieve small chunks of hex data from the hex string</summary>
    /// <param name="returnHex">: Provide the string that will store the hex bytes</param>
    /// <param name="chunk">: Include this if you would like to get data from a specific chunk</param>
    /// <param name="length">: The amount of hex pairs to return I.E. 1 -> FF</param>
    /// <param name="startingPoint">: Starting position of the chunk, defaults to 0</param>
    static void getSmallHexChunk( string& returnHex, const string& chunk, int length, int startingPoint = 0);

    void getChunkData(string& data, string& type, std::map<string, string>& chunkData);

    /// <summary>Returns the decimal value of the passed hex string</summary>
    /// <param name="hexChunk">: The hex chunk you want the value of</param>
    static int getHexValue(const string& hexChunk);

private:
    int currentPos = 0;
};