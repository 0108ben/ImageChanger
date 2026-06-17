#pragma once

#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <map>
#include <unordered_map>
#include <cmath>
#include <array>
#include <list>

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

static const string hexsRGB = "73 52 47 42";
static const string hexsBIT = "73 42 49 54";

static const string hextEXt = "74 45 58 74";

struct HuffmanCode
{
    int bits;
    int minBinary;
    int maxBinary;
    int minSymbol;
    int maxSymbol;
};

struct Pixel
{
    int red;
    int green;
    int blue;
    int alpha;
};

struct ImageData
{
    static const std::unordered_map<int, string> colourType;

    static const std::unordered_map<int, string> renderingIntent;

    static const std::unordered_map<string, string> compressionType;

    static const std::array<HuffmanCode, 4> huffmanCodes;

    bool endOfBytes = false;

    string hexString;
    std::vector<std::map<string, string>> chunks{};

    string signature;
    int imageWidth{};
    std::vector<int> pixelData;
    std::vector<Pixel> image;


    explicit ImageData(const string& imageLocation);

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
    /// <param name="keyWord">: If tEXt keyword chunk then set to true (Needs to identify null seperator)</param>
    int getSmallHexChunk( string& returnHex, const string& chunk, int length, int startingPoint = 0, bool keyword = false);

    /// <summary>Used to retrieve the data of a chunk</summary>
    /// <param name="chunk">: The chunk to search for data</param>
    /// <param name="type">: The chunk type to know what data to search for</param>
    /// <param name="chunkData">: Stores all the data found from the chunk</param>
    void getChunkData(const string& chunk, const string& type, std::map<string, string>& chunkData, const int dataLength = 0);

    /// <summary>Used to decode the IDAT data and store each pixels data within image(variable)</summary>
    /// <param name="binaryData">: The IDAT data to be read</param>
    /// <param name="compressionVal">: The compression value gained from huffmanSequence</param>
    void readImageDataBinary(string& binaryData, const string& compressionVal);

    /// <summary>Prints out all the info of a chunk to the console</summary>
    void displayChunk(std::map<string, string>& chunk);

    /// <summary>Returns the decimal value of the passed hex string</summary>
    /// <param name="hexChunk">: The hex chunk you want the value of</param>
    static int getHexValue(const string& hexChunk);

    /// <summary>Returns the decimal value of the passed binary string</summary>
    /// <param name="binaryChunk">: The binary chunk you want the value of</param>
    static int getBinaryValue(const string &binaryChunk);

    /// <summary>Returns the ascii value of the passed hex string</summary>
    /// <param name="hexChunk">: The hex chunk you want the ascii value of</param>
    static string getHexASCII(const string& hexChunk);

    /// <summary>Returns the binary value of the passed hex string</summary>
    /// <param name="hexChunk">: The hex chunk you want the binary value of</param>
    /// <param name="LSB">: Default false, if true the binary will be in LSB format</param>
    static string getHexBinary(const string& hexChunk, bool LSB = false);

private:
    int currentPos = 0;

    int colourTypeVal = 0;

    static int getHexASCIIValue(char hexChar);
};