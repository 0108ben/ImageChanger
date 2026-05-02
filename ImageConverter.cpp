#include "ImageConverter.hpp"

const std::unordered_map<int, string> ImageData::colourType =
{
    {0, "Greyscale"},
    {2, "Truecolour"},
    {3, "Indexed-colour"},
    {4, "Greyscale with alpha"},
    {6, "Truecolour with alpha"}
};

const std::unordered_map<int, string> ImageData::renderingIntent =
{
    {0, "Perceptual"},
    {1, "Relative colorimetric"},
    {2, "Saturation"},
    {3, "Absolute colorimetric"}
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

    // IEND has no data, need to test this
    getSmallHexChunk(chunkData, getHexValue(chunk["length"]));
    chunk["data"] = chunkData;

    getSmallHexChunk(CRC, 4);
    chunk["CRC"] = CRC;

    getChunkData(chunk["data"], chunk["type"], chunk);
}

void ImageData::getSmallHexChunk(string& returnHex, const int length, int startingPoint)
{
    // If len is 1 -> return 2 chars 00, FF, etc.
    // Each hex value has a white space inbetween -> 00 11 -> return 5 chars if len is 2, 8 chars if len is 3

    // If < 0 likely IEND chunk
    if (length < 1)
        return;

    const int characters = length * 2 + (length - 1);

    bool defaultValue = false;

     startingPoint == -1 ? startingPoint = currentPos, defaultValue = true : defaultValue = false;

    if (!defaultValue)
        startingPoint *= 3;

    returnHex = hexString.substr(startingPoint, characters);

    if (defaultValue)
    {
        currentPos += characters + 1;

        if (currentPos == hexString.length())
        {
            endOfBytes = true;
        }
    }
}

void ImageData::getSmallHexChunk(string &returnHex, const string &chunk, const int length, int startingPoint)
{
    // If len is 1 -> return 2 chars 00, FF, etc.
    // Each hex value has a white space inbetween -> 00 11 -> return 5 chars if len is 2, 8 chars if len is 3

    const int characters = length * 2 + (length - 1);

    startingPoint *= 3;

    returnHex = chunk.substr(startingPoint, characters);
}

void ImageData::getChunkData(const string &chunk, const string &type, std::map<string, string> &chunkData)
{
    if (type == hexIHDR)
    {
        string width, height, bitDepth, colourType, compression, filter, interlace;
        getSmallHexChunk(width, chunk, 4);
        chunkData["width"] = width;
        getSmallHexChunk(height, chunk, 4, 4);

        chunkData["height"] = height;
        getSmallHexChunk(bitDepth, chunk, 1, 8);

        chunkData["bitDepth"] = bitDepth;
        getSmallHexChunk(colourType, chunk, 1, 9);

        chunkData["colourType"] = colourType;
        getSmallHexChunk(compression, chunk, 1, 10);

        chunkData["compression"] = compression;
        getSmallHexChunk(filter, chunk, 1, 11);

        chunkData["filter"] = filter;
        getSmallHexChunk(interlace, chunk, 1, 12);

        chunkData["interlace"] = interlace;
    }

    else if (type == hexsRGB)
    {
        string renderingIntentVal;
        getSmallHexChunk(renderingIntentVal, chunk, 1);
        chunkData["renderingIntent"] = renderingIntentVal;
    }

    else if (type == hexIDAT)
    {

    }

    else if (type == hexIEND)
    {
        return;
    }

    else
    {
        cout << "Invalid chunk type: " << type << endl;
        exit(-1);
    }
}

void ImageData::displayChunk(std::map<string, string> &chunk)
{
    cout << "\n\n";

    if (chunk["type"] == hexIHDR)
    {
        cout << "Chunk size: " << chunk["length"] << " (" << getHexValue(chunk["length"]) << ")" << endl;
        cout << "Chunk type: " << chunk["type"]  << " (IHDR)" << endl;

        cout << "Width: " << chunk["width"] << " (" << getHexValue(chunk["width"]) << ")" << endl;
        cout << "Height: " << chunk["height"] << " (" << getHexValue(chunk["height"]) << ")" << endl;
        cout << "Bit depth: " << chunk["bitDepth"] << endl;
        cout << "Colour type: " << chunk["colourType"] << " (" << colourType.find(getHexValue(chunk["colourType"]))->second << ")" << endl;
        cout << "Compression: " << chunk["compression"] << endl;
        cout << "Filter: " << chunk["filter"] << endl;
        cout << "Interlace: " << chunk["interlace"] << endl;

        cout << "CRC: " << chunk["CRC"] << endl;
    }

    else if (chunk["type"] == hexsRGB)
    {
        cout << "Chunk size: " << chunk["length"] << " (" << getHexValue(chunk["length"]) << ")" << endl;
        cout << "Chunk type: " << chunk["type"]  << " (sRGB)" << endl;

        cout << "Rendering Intent: " << chunk["renderingIntent"] << " (" << renderingIntent.find(getHexValue(chunk["renderingIntent"]))->second << ") " << endl;

        cout << "CRC: " << chunk["CRC"] << endl;
    }

    else if (chunk["type"] == hexIDAT)
    {
        cout << "Chunk size: " << chunk["length"] << " (" << getHexValue(chunk["length"]) << ")" << endl;
        cout << "Chunk type: " << chunk["type"]  << " (IDAT)" << endl;

        cout << "Image Info: " << chunk["data"] << endl;

        cout << "CRC: " << chunk["CRC"] << endl;
    }

    else if (chunk["type"] == hexIEND)
    {
        cout << "Chunk size: " << chunk["length"] << " (" << getHexValue(chunk["length"]) << ")" << endl;
        cout << "Chunk type: " << chunk["type"]  << " (IEND)" << endl;

        cout << "Data: " << "NO DATA (IEND)" << endl;

        cout << "CRC: " << chunk["CRC"] << endl;
    }

    else
    {
        cout << "Invalid chunk type: " << chunk["type"] << endl;
        exit(-1);
    }
}

int ImageData::getHexValue(const string &hexChunk)
{
    auto hexVal = hexChunk;
    std::erase_if(hexVal, isspace);

    return std::stoi(hexVal, nullptr, 16);
}
