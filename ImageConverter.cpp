#include "ImageConverter.hpp"

const std::unordered_map<int, string> ImageData::colourType =
{
    {0, "Greyscale"},
    {2, "Truecolour"},
    {3, "Indexed-colour"},
    {4, "Greyscale with alpha"},
    {6, "Truecolour with alpha"}
};


void ImageData::convertImageToHex(const string& imageLocation)
{
    ifstream imageOne(imageLocation, std::ios::binary);

    std::vector<unsigned char> imageBytes((std::istreambuf_iterator<char>(imageOne)), std::istreambuf_iterator<char>());

    imageOne.close();

    std::stringstream ss;
    for (auto byte : imageBytes)
    {
        ss << hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    }

    hexString = ss.str();
}

void ImageData::getHexChunk(std::map<string, string>& chunk)
{
    string chunkLength, chunkType, chunkData, CRC;

    getSmallHexChunk(chunkLength, 4);
    chunk["length"] = chunkLength;

    getSmallHexChunk(chunkType, 4);
    chunk["type"] = chunkType;

    getSmallHexChunk(chunkData, getHexValue(chunk["length"]));
    chunk["data"] = chunkData;

    getSmallHexChunk(CRC, 4);
    chunk["CRC"] = CRC;
}

void ImageData::getSmallHexChunk(string& returnHex, const int length, int startingPoint)
{
    // If len is 1 -> return 2 chars 00, FF, etc.
    // Each hex value has a white space inbetween -> 00 11 -> return 5 chars if len is 2, 8 chars if len is 3

    const int characters = length * 2 + (length - 1);

    bool defaultValue = false;

     startingPoint == -1 ? startingPoint = currentPos, defaultValue = true : defaultValue = false;

    if (!defaultValue)
        startingPoint *= 3;

    returnHex = hexString.substr(startingPoint, characters);

    if (defaultValue)
        currentPos += characters + 1;
}

void ImageData::getSmallHexChunk(string &returnHex, const string &chunk, const int length, int startingPoint)
{
    // If len is 1 -> return 2 chars 00, FF, etc.
    // Each hex value has a white space inbetween -> 00 11 -> return 5 chars if len is 2, 8 chars if len is 3

    const int characters = length * 2 + (length - 1);

    startingPoint *= 3;

    returnHex = chunk.substr(startingPoint, characters);
}

void ImageData::getChunkData(string &data, string &type, std::map<string, string> &chunkData)
{
    if (type == hexIHDR)
    {
        string width, height, bitDepth, colourType, compression, filter, interlace;
        getSmallHexChunk(width, data, 4);
        chunkData["width"] = width;
        getSmallHexChunk(height, data, 4, 4);

        chunkData["height"] = height;
        getSmallHexChunk(bitDepth, data, 1, 8);

        chunkData["bitDepth"] = bitDepth;
        getSmallHexChunk(colourType, data, 1, 9);

        chunkData["colourType"] = colourType;
        getSmallHexChunk(compression, data, 1, 10);

        chunkData["compression"] = compression;
        getSmallHexChunk(filter, data, 1, 11);

        chunkData["filter"] = filter;
        getSmallHexChunk(interlace, data, 1, 12);

        chunkData["interlace"] = interlace;
    }

    else if (type == hexIDAT)
    {

    }

    else if (type == hexIEND)
    {

    }

    else
    {
        cout << "Invalid chunk type: " << type << endl;
        exit(-1);
    }
}

int ImageData::getHexValue(const string &hexChunk)
{
    auto hexVal = hexChunk;
    std::erase_if(hexVal, isspace);

    return std::stoi(hexVal, nullptr, 16);
}
